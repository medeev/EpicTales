////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		룸스레드
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Core/Task/TaskManager.h>
#include <Core/Thread/IThread.h>

#include "RoomTypes.h"

class RoomThread
	:
	public Core::IThread
{
	/// 룸 맵 타입 정의
	typedef std::unordered_map< uint64_t, RoomPtr > Rooms;

public:
	/// 콜백 타입 정의
	typedef std::function< void(RoomPtr) > Callback;

private:
	uint32_t          _index;		      ///< 인덱스
	Rooms		      _rooms;	          ///< 방 목록
	std::atomic_int	  _roomCount;	      ///< 방 개수
	std::atomic_int	  _reservedRoomCount; ///< 예약된 방 개수
	Core::TaskManager _taskManager;       ///< 작업 관리자

public:
	/// 생성자
	RoomThread(uint32_t index);

	/// 소멸자
	~RoomThread();

	/// 순회한다
	void forEach(const Callback& callback);

	/// 방 개수를 반환한다
	uint32_t getRoomCount(bool withReserved = true) const;

	/// 해당 스레드에서 작업을 실행한다
	void runTask(const Core::TaskCaller& taskCaller, const Core::TaskManager::Task& task);

	/// 룸을 추가한다
	void addRoom(RoomPtr room);

public:
	/// 스레드 코드를 실행한다
	virtual void onRun() override;
};
