


#include "Pch.h"
#include "RoomThread.h"

#include "Room.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
RoomThread::RoomThread(uint32_t index)
	:
	IThread("Room"),
	_index(index),
	_taskManager("RoomThread", *this),
	_roomCount(0),
	_reservedRoomCount(0)
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	소멸자
///
///	@ return 
////////////////////////////////////////////////////////////////////////////////////////////////////
RoomThread::~RoomThread()
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		순회한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void RoomThread::forEach(const Callback& callback)
{
	for (auto& it : _rooms)
		callback(it.second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		방 수를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t RoomThread::getRoomCount(bool withReserved) const
{
	uint32_t roomCount = _roomCount.load();
	if (withReserved)
		roomCount += _reservedRoomCount.load();

	return roomCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		해당 스레드에서 작업을 실행한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void RoomThread::runTask(const Core::TaskCaller& taskCaller, const Core::TaskManager::Task& task)
{
	_taskManager.runTask(taskCaller, task);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스레드 코드를 실행한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void RoomThread::onRun()
{
	static std::atomic_int ms_seed;
	srand((uint32_t)(Core::Time::GetCurTimeMSec()) + ms_seed.fetch_add(1));

	const static int32_t fps = (int32_t)(1000 / 30);
	int64_t oldMSec = Core::Time::GetCurTimeMSec();

	while (_running)
	{
		int64_t newMSec = Core::Time::GetCurTimeMSec();
		int32_t timeDiff = (int32_t)(newMSec - oldMSec);
		if (timeDiff <= 0)
		{
			Core::IThread::Sleep(fps);
			continue;
		}

		if (timeDiff > 1000 * 15)
		{
			WARN_LOG("timeDiff is too big. %d msec]", timeDiff);
			oldMSec = newMSec;
			continue;
		}

		_taskManager.update(newMSec);

		for (auto it = _rooms.begin(); it != _rooms.end(); )
		{
			auto& room = it->second;
			room->setDeltaMSec(timeDiff);
			room->setTime(newMSec);
			room->update();
			if (
				room->isRoomRemovable() &&
				room->getTaskManager()->getTaskCount() <= 0)
			{
				if (room->getPlayerCount() > 0)
				{
					++it;
					continue;
				}

				it = _rooms.erase(it);
				_roomCount.store((const int)_rooms.size());
			}
			else
			{
				++it;
			}
		}

		int32_t elapsedMsec = (int32_t)(Core::Time::GetCurTimeMSec() - newMSec);
		int32_t delayMSec = Core::Numeric::Max<int32_t>(1, fps - elapsedMsec);
		IThread::Sleep(delayMSec);
		oldMSec = newMSec;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		룸을 추가한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void RoomThread::addRoom(RoomPtr room)
{
	if (!room)
		return;

	_reservedRoomCount.fetch_add(1);
	runTask(
		Caller,
		[this, room]()
		{
			_reservedRoomCount.fetch_sub(1);
			_rooms[room->getId()] = room;
			_roomCount.store((const int)_rooms.size());
		});
}
