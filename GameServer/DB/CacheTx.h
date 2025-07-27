
#pragma once

#include <Core/Task/TaskCaller.h>
#include <DB/OrmObject.h>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <unordered_map>

#include "BusyState.h"

namespace Core {
	class TaskManager;
	typedef std::shared_ptr<TaskManager> TaskManagerPtr;
};

class CacheTx : public std::enable_shared_from_this<CacheTx>
{
	enum class ETxReturnType
	{
		Succ,
		FailLogic,
		FailTransation
	};
	CacheTx() = delete;
	CacheTx& operator=(const CacheTx&) = delete;

public:
	enum class ETxSyncType
	{
		AsyncDB_AsyncCallback, ///< 비동기로 디비 처리, 디비처리후 비동기로 콜백호출
		SyncCallback_AsyncDB,  ///< 동기적으로 콜백 먼저 호출하고 디비는 나중에 비동기로 처리 (DB는 성공으로 미리판단, 메모리미리반영)
		SyncCallback_ManualDB, ///< 동기적으로 콜백 먼저 호출하고 디비는 나중에 수동으로 호출해서 처리
	};

private:
	typedef std::pair<DBOrm::OrmObjectPtr/*사본*/, DBOrm::OrmObjectPtr/*원본*/> CacheOrignPair;

	Core::TaskCaller _taskCaller;
	ETxSyncType _syncType;
	std::unordered_map<DBOrm::OrmObjectPtr/*원본*/, DBOrm::OrmObjectPtr/*사본*/ > _objects; ///< db에서 수행해야할 객체수행오브젝트목록
	std::unordered_map<Core::TaskManagerPtr, std::list<CacheOrignPair>> _objectThds; ///< 사본데이터가 어디서 왔고 다시반영야하는 작업스레드
	std::list<std::pair<Core::TaskManagerPtr, std::function<bool(void)>>> _tasks; ///< 작업스레드에서 수행해야할 목록
	std::list<std::function<bool(const DataBaseTarget& db)>> _dbTasks; ///< 추가 db작업목록
	std::unordered_map<Core::TaskManagerPtr, std::list<std::function<void()>>> _succTasks; // 성공시 호출되는 콜백 목록
	std::unordered_map<Core::TaskManagerPtr, std::list<std::function<void()>>> _failTasks; // 실패시 호출되는 콜백 목록
	std::unordered_map<Core::TaskManagerPtr, std::list<std::function<void(bool)>>> _resultTasks; // 성공/실패 둘다에 콜백 목록
	std::list<BusyStateUpdaterPtr> _busyUpdaters;
	ETxReturnType _returnType;

public:
	CacheTx(const Core::TaskCaller& taskCaller);
	~CacheTx();

	/// TaskManager를 포함하는 개체를 위한 템플릿함수 시작
	template<typename T, typename TaskOwnerType>
	std::shared_ptr<T> acquireObject(
		TaskOwnerType& taskOwner, const std::shared_ptr<T> ormPtr)
	{
		Core::TaskManagerPtr taskManager = taskOwner.getTaskManager();
		return acquireObject(taskManager, ormPtr);
	}
	template<typename TaskOwnerType>
	void pushTask(TaskOwnerType& taskOwner, std::function<bool(void)> task) {
		Core::TaskManagerPtr taskManager = taskOwner.getTaskManager();
		pushTask(taskManager, task);
	}
	template<typename TaskOwnerType>
	void ifSucceed(TaskOwnerType& taskOwner, std::function<void()> callback) {
		Core::TaskManagerPtr taskManager = taskOwner.getTaskManager();
		ifSucceed(taskManager, callback);
	}
	template<typename TaskOwnerType>
	void ifFailed(TaskOwnerType& taskOwner, std::function<void()> callback) {
		Core::TaskManagerPtr taskManager = taskOwner.getTaskManager();
		ifFailed(taskManager, callback);
	}
	template<typename TaskOwnerType>
	void ifResult(TaskOwnerType& taskOwner, std::function<void(bool)> callback) {
		Core::TaskManagerPtr taskManager = taskOwner.getTaskManager();
		ifResult(taskManager, callback);
	}
	/// TaskManager를 포함하는 개체를 위한 템플릿함수 끝

	template<typename T>
	const std::shared_ptr<T> findObject(
		const std::shared_ptr<T> ormPtr) const
	{
		auto iter = _objects.find(ormPtr);
		return (iter == _objects.end()) ? nullptr : std::dynamic_pointer_cast<T>(iter->second);
	}

	
	void pushDbTask(std::function<bool(const DataBaseTarget& db)> task);
	void run();
	
	void addBusyUpdater(const BusyStateUpdaterPtr& busy) {
		if (!busy)
			return;
		_busyUpdaters.push_back(busy);
	}
	void setSyncType(ETxSyncType type) {
		_syncType = type;
	}

protected:
	template<typename T>
	std::shared_ptr<T> acquireObject(
		Core::TaskManagerPtr taskManager, const std::shared_ptr<T> ormPtr)
	{
		auto iter = _objects.find(ormPtr);
		if (iter != _objects.end())
			return std::dynamic_pointer_cast<T>(iter->second);

		auto cacheObj = ormPtr->clone();

		_objects.insert(std::make_pair(ormPtr, cacheObj));

		if (taskManager != nullptr)
			_objectThds[taskManager].push_back(std::make_pair(cacheObj, ormPtr));

		return cacheObj;
	}
	void pushTask(Core::TaskManagerPtr taskManager, std::function<bool(void)> task);
	void ifSucceed(Core::TaskManagerPtr taskManager, std::function<void()> callback);
	void ifFailed(Core::TaskManagerPtr taskManager, std::function<void()> callback);
	void ifResult(Core::TaskManagerPtr taskManager, std::function<void(bool)> callback);
private:
	void _runOnDbThd();
	void _runPost();
	void _runFailTasks();
};

typedef std::shared_ptr<CacheTx> CacheTxPtr;
typedef std::weak_ptr<CacheTx> CacheTxWeakPtr;
