#include "Pch.h"
#include "CacheTx.h"

#include <Core/Task/TaskManager.h>
#include <DB/DBTask.h>
#include <DB/Orms.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CacheTx::CacheTx(const Core::TaskCaller& taskCaller)
	: _taskCaller(taskCaller), _returnType(ETxReturnType::Succ), _syncType(ETxSyncType::AsyncDB_AsyncCallback)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
CacheTx::~CacheTx()
{
	if (!_tasks.empty())
		WARN_LOG("_task not empty");
	if (!_dbTasks.empty())
		WARN_LOG("_dbTasks not empty");
	if (!_succTasks.empty())
		WARN_LOG("_succTasks not empty");
	if (!_failTasks.empty())
		WARN_LOG("_failTasks not empty");
	if (!_resultTasks.empty())
		WARN_LOG("_resultTasks not empty");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		작업을 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CacheTx::pushTask(Core::TaskManagerPtr taskManager, std::function<bool(void)> task)
{
	ENSURE(taskManager, return);

	_tasks.push_back(std::make_pair(taskManager, task));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  db작업을 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CacheTx::pushDbTask(std::function<bool(const DataBaseTarget& db)> task)
{
	_dbTasks.push_back(task);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		작업을 수행한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CacheTx::run()
{
	if (
		_tasks.empty() &&
		_objects.empty() &&
		_objectThds.empty() &&
		_dbTasks.empty() &&
		_succTasks.empty() &&
		_failTasks.empty() &&
		_resultTasks.empty())
	{
		WARN_LOG("empty task and object change");
		return;
	}

	auto self = shared_from_this();
	std::shared_ptr<int32_t> dispose = std::shared_ptr<int32_t>(new int32_t(0),
		[self, this](int32_t* dispose)
		{
			if ((*dispose) == 0)
			{
				if (_syncType == ETxSyncType::AsyncDB_AsyncCallback)
				{
					_runOnDbThd();
				}
				else if (_syncType == ETxSyncType::SyncCallback_AsyncDB)
				{
					_runPost();
					_runOnDbThd();
				}
				else if (_syncType == ETxSyncType::SyncCallback_ManualDB)
				{
					_runPost();
				}
			}
			else
			{
				_returnType = ETxReturnType::FailLogic;
				_runFailTasks();
				return;
			}
		});

	for (auto& taskPair : _tasks)
	{
		const auto taskManager = taskPair.first;
		ENSURE(taskManager, continue);

		const auto& task = taskPair.second;

		taskManager->diapatchTask(_taskCaller, [task, dispose]()
			{
				if (!task())
					++(*dispose);
			});
	}
	_tasks.clear();

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  성공할때 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CacheTx::ifSucceed(Core::TaskManagerPtr taskManager, std::function<void()> callback)
{
	ENSURE(taskManager, return);

	_succTasks[taskManager].push_back(callback);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  실패할때 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CacheTx::ifFailed(Core::TaskManagerPtr taskManager, std::function<void()> callback)
{
	ENSURE(taskManager, return);

	_failTasks[taskManager].push_back(callback);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  결과시 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CacheTx::ifResult(Core::TaskManagerPtr taskManager, std::function<void(bool)> callback)
{
	ENSURE(taskManager, return);

	_resultTasks[taskManager].push_back(callback);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		DB에서 작업을 수행한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CacheTx::_runOnDbThd()
{
	auto self = shared_from_this();
	auto resultCallback = [self, this](bool result)
		{
			if (!result)
				_returnType = ETxReturnType::FailTransation;

			if (_syncType == ETxSyncType::AsyncDB_AsyncCallback)
				_runPost();
		};

	if (_objects.empty() && _dbTasks.empty())
	{
		resultCallback(true);
		return;
	}

	DBTask::Instance().runTask(_taskCaller, [self, this](const DataBaseTarget& db)
		{
			for (auto& task : _dbTasks)
			{
				if (!task(db))
					return false;
			}
			_dbTasks.clear();

			for (auto& objPair : _objects)
			{
				auto ormObj = objPair.second;
				switch (ormObj->getCrudState())
				{
				case DBOrm::OrmObject::CRUD::Insert:
					if (!ormObj->insert(db))
						return false;
					break;
				case DBOrm::OrmObject::CRUD::Update:
					if (!ormObj->update(db))
						return false;
					break;
				case DBOrm::OrmObject::CRUD::Delete:
					if (!ormObj->deleteKey(db))
						return false;
					break;
				default:
					break;
				}
			}

			return true;

		}, resultCallback, EProjDBType::Game, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		작업이 끝난 사본객체로 원본객체에 반영한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CacheTx::_runPost()
{
	if (_returnType != ETxReturnType::Succ)
	{
		_runFailTasks();

		return;
	}

	auto self = shared_from_this();
	std::shared_ptr<void> dispose = std::shared_ptr<void>(
		nullptr,
		[self, this](void*)
		{
			for (auto& taskPair : _succTasks)
			{
				const auto& taskManager = taskPair.first;
				ENSURE(taskManager, continue);

				const auto& tasks = taskPair.second;

				taskManager->diapatchTask(_taskCaller, [tasks]()
					{
						for (auto task : tasks)
							task();
					});
			}
			for (auto& taskPair : _resultTasks)
			{
				const auto& taskManager = taskPair.first;
				ENSURE(taskManager, continue);

				const auto& tasks = taskPair.second;

				taskManager->runTask(_taskCaller, [tasks]()
					{
						for (auto task : tasks)
							task(true);
					});
			}

			_resultTasks.clear();
			_succTasks.clear();
			_failTasks.clear();
		});

	for (auto& objPair : _objectThds)
	{
		auto taskManager = objPair.first;
		ENSURE(taskManager, continue);

		const auto& ormObjPair = objPair.second;

		taskManager->diapatchTask(_taskCaller, [ormObjPair, dispose]()
			{
				for (auto& pairCopyOrigin : ormObjPair)
				{
					auto copy = pairCopyOrigin.first;
					auto origin = pairCopyOrigin.second;

					// 1. 커스텀처리를 먼저한다.
					copy->applyCache(origin);

					// 2. 원본에 적용한다.
					copy->copyMember(origin);
				}
			});
	}

	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  실패했을때 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CacheTx::_runFailTasks()
{
	for (auto& taskPair : _failTasks)
	{
		const auto& taskManager = taskPair.first;
		ENSURE(taskManager, continue);

		const auto& tasks = taskPair.second;

		taskManager->runTask(_taskCaller, [tasks]()
			{
				for (auto task : tasks)
					task();
			});
	}
	for (auto& taskPair : _resultTasks)
	{
		const auto& taskManager = taskPair.first;
		ENSURE(taskManager, continue);

		const auto& tasks = taskPair.second;

		taskManager->runTask(_taskCaller, [tasks]()
			{
				for (auto task : tasks)
					task(false);
			});
	}

	_resultTasks.clear();
	_failTasks.clear();
	_succTasks.clear();
	_dbTasks.clear();
}
