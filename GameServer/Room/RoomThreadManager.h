////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		룸스레드 관리자
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Core/Util/Singleton.h>

#include "RoomThread.h"

class RoomThreadManager
	:
	public Core::Singleton<RoomThreadManager>
{
	friend class Core::Singleton<RoomThreadManager>;
	/// 룸스레드 목록 타입 정의
	typedef std::vector<std::shared_ptr<RoomThread>> RoomThreads;

private:
	RoomThreads	_threads; ///< 룸스레드 목록

private:
	RoomThreadManager() {}
	~RoomThreadManager() {}

public:
	/// 초기화 한다
	void initialize(uint32_t threadCount);

	/// 마무리 한다
	void finalize();

	/// 한가한 스레드를 반환한다
	std::shared_ptr<RoomThread> getIdleThread();

	/// 순회한다
	void forEach(const RoomThread::Callback& callback);
};
