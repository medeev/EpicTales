////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		전송 스레드 관리자
///
///	@ date		2024-3-7
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once 


#include <Core/Util/Singleton.h>

#include "User/User.h"

class SendThread;

class SendThreadManager
	:
	public Core::Singleton<SendThreadManager>
{
	friend class Core::Singleton<SendThreadManager>;
	///< 전송 스레드 목록 타입 정의
	typedef std::vector< SendThread* > Threads;

private:
	Threads	_threads; ///< 스레드 목록
	uint32_t _index;   ///< 인덱스

private:
	SendThreadManager();
	virtual ~SendThreadManager();

public:
	/// 초기화 한다
	void initialize();

	/// 해제한다.
	void finalize();

	/// 패킷을 보낸다
	void send(UserPtr userPtr, NaglePacketPackPtr naglePacket);
};
