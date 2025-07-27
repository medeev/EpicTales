

#include "Pch.h"
#include "LogManager.h"

#include <Task/TaskCaller.h>
#include <Util/IdFactory.h>

#include "Config.h"
#include "DB/DBTask.h"

static LogManager g_instanceLogManager;		///< 로그 관리자 인스턴스


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화 한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void LogManager::InsertPacketLog(
	uint64_t userId,
	uint64_t playerId,
	const Core::Time& time,
	const Core::Time& virtualTime,
	int8_t type,
	const PktBase& packet)
{
	if (!Config::GetPacketDBLog())
		return;

	//DBTask::GetInstance().runTask(
	//	Caller,
	//	[ = ](OdbcConnPtr conn )
	//	{
	//		PacketLogOrmObject packetLog;
	//		packetLog.id = Core::IdFactory::CreateId();
	//		packetLog.playerId = playerId;
	//		packetLog.time = time;
	//		packetLog.virtualTime = virtualTime;
	//		packetLog.type = type;
	//		packetLog.packetId = packetId;

	//		packetLog.packetName = packetName;
	//		packetLog.packetBody = packetBody;

	//		auto ret = packetLog.queryInsert( mysql );
	//		return ret.success;
	//		

	//		return true;

	//	}, EDBServerType::Log );
}
