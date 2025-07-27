////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		로그 관리자
///
///	@ date		2024-3-7
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Protocol/PktBase.h"

class LogManager
	:
	public Core::Singleton<LogManager>
{
public:
	/// 패킷 로그를 추가한다.
	static void InsertPacketLog(
		uint64_t userId,
		uint64_t playerId,
		const Core::Time& time,
		const Core::Time& virtualTime,
		int8_t   type,
		const PktBase& packet);
};
