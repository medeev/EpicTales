

#include "Pch.h"
#include "User.h"

#include <Core/Util/AppInstance.h>
#include <Core/Thread/IThread.h>
#include <Asio/Service/NetBuffer.h>
#include <Protocol/IPacketHandler.h>
#include <Protocol/PktBase.h>
#include <Protocol/PacketHeader.h>
#include <Protocol/PacketReader.h>
#include <Protocol/Struct/PktTypes.h>

#include "UserManager.h"
#include "Actor/Player.h"
#include "Config.h"
#include "Log/LogManager.h"
#include "Logic/SendThreadManager.h"
#include "Logic/PlayerManager.h"
#include "Room/WorldManager.h"
#include "Logic/Character/CloseComponent.h"
#include "Logic/Character/CharacterComponent.h"
#include "Room/RoomThreadContext.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		패킷 로그를 남긴다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void InsertPacketLog(int8_t type, uint64_t userId, uint64_t playerId, const PktBase& packet)
{
	if (!Config::GetPacketDBLog())
		return;

	switch ((PacketId)(packet.getPacketId()))
	{

	case PacketId::PktPingReq:
		return;
	default:
		break;
	}

	auto curTime = Core::Time::GetCurTime(false);
	auto virtualTime = curTime + Core::TimeSpan(Core::Time::GetExtraMillis());
	LogManager::InsertPacketLog(userId, playerId, curTime, virtualTime, type, packet);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
User::User(int32_t connectionId)
	:
	super(connectionId),
	_packetQueue(512),
	_packetQueueForNotCheckBusy(512)
{
	_id = 0;
	_packetVersion = 0;
	_numbering = 0;
	_versionCheck = false;
	_lastPingTime = 0;
	_latency = 0;
	_latencyCount = 0;
	_latencyCheckTime = 0;
	_isCleanCloseData = false;
	_isCustomNagle = false;
	_isCloseProcessed = false;

	_naglePacket.store(std::make_shared<NaglePacketPack>());
	_busyState = std::make_shared<BusyState>();

	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
User::~User()
{
	_packetQueue.consume_all([](PktBase* packet) {
		Core::Generic::Delete(packet);
		});
	_packetQueueForNotCheckBusy.consume_all([](PktBase* packet) {
		Core::Generic::Delete(packet);
		});

	if (auto naglePacket = _naglePacket.load())
	{
		naglePacket->queue.consume_all([](NaglePacket* packet) {
			Core::Generic::Delete(packet);
			});
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  패킷큐를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
Core::BoostLFQ<PktBase*>& User::getPacketQueue(PktBase& packet)
{
	switch ((PacketId)(packet.getPacketId()))
	{
	case PacketId::PktPingReq:
	case PacketId::PktVersionReq:
	case PacketId::PktPlayerMoveReq:
	case PacketId::PktChatReq:
	case PacketId::PktEnvObjCtrlStartReq:
	case PacketId::PktEnvObjCtrlCancelReq:
		return _packetQueueForNotCheckBusy;
	}

	return _packetQueue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		플레이어를 설정한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::possess(PlayerPtr player)
{
	if (player)
	{
		player->setUser(getUserPtr());
		player->getCloseComponent().possess();
	}

	_playerWeakPtr = player;

	INFO_LOG("possess [uid:%llu, pid:%llu]",
		getId(), player ? player->getId() : 0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  접속이 끊어질때 플레이어 소유권을 초기화한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::unPossess()
{
	auto player = getPlayer();
	INFO_LOG("up possess [uid:%llu, pid:%llu]", getId(), player ? player->getId() : 0);

	if (player)
	{
		player->getCloseComponent().unPossess(*this);
		player->setUser(nullptr);
	}

	_playerWeakPtr.reset();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		대기 룸을 설정한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::setWaitRoom(UserWaitRoomPtr room)
{
	_waitRoom = room;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		대기룸에 입장한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::enterWaitRoom()
{
	/*UserWaitRoomPtr waitRoom = std::dynamic_pointer_cast<UserWaitRoom>(
		UserWaitRoom::Instance().shared_from_this());
	setWaitRoom(waitRoom);
	waitRoom->addUser(getUserPtr());*/

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		대기 룸에서 나온다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::leaveWaitRoom()
{
	if (!_waitRoom.lock())
		return;

	/*UserWaitRoomPtr waitRoom = std::dynamic_pointer_cast<UserWaitRoom>(
		UserWaitRoom::GetInstance().shared_from_this());
	setWaitRoom(nullptr);
	waitRoom->removeUser(getId());*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		플레이어를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
PlayerPtr User::getPlayer()
{
	return _playerWeakPtr.lock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		플레이어를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
const PlayerPtr User::getPlayer() const
{
	return _playerWeakPtr.lock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  강제로 접속종료 시키고 정리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::forceClose()
{
	_closeProcess(); /// 미리해서 소켓 Close이벤트 발생시 이후 처리되는 로직으로 새접속에 영향을 미치는것을 차단한다.
	closeSocket();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		패킷을 보낸다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool User::send(const PktBase& pktBase)
{
	uint64_t playerId = 0;
	if (auto player = getPlayer())
		playerId = player->getId();
	InsertPacketLog(2, getId(), playerId, pktBase);

	PacketHeader pktHeader(_numbering);

	if (Config::GetSendThreadCount() && isCustomNagle())
	{
		PacketWriterPtr writer = std::make_shared<PacketWriter>();
		writer->setVersion(_packetVersion);
		pktHeader.generate(pktBase, *writer, false);

		if (auto naglePacket = _naglePacket.load())
		{
			naglePacket->totalSize.fetch_add(writer->getLength());
			naglePacket->queue.push(new NaglePacket(writer));
		}
		return true;
	}

	PacketWriter writer;
	writer.setVersion(_packetVersion);
	pktHeader.generate(pktBase, writer, false);

	return super::send(writer.getBuffer(), writer.getLength());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		네이글 패킷을 보낸다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::sendNaglePacket()
{
	if (!isCustomNagle())
		return;

	if (auto  naglePacket = _naglePacket.load())
	{
		if (naglePacket->totalSize <= 0)
			return;

		_naglePacket.store(std::make_shared<NaglePacketPack>());
		SendThreadManager::Instance().send(getUserPtr(), naglePacket);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::consumePacketQueue()
{
	while (true)
	{
		if (!_packetQueueForNotCheckBusy.consume_one(_packetQueueDispose))
			break;
	}

	while (true)
	{
		if (_busyState->getState() == BusyState::EState::kBusy)
			break;

		if (!_packetQueue.consume_one(_packetQueueDispose))
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		패킷을 추출한다
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t User::_onExtractPacket(uint8_t* data, uint32_t len, bool& result)
{
	const uint32_t intactSize = PacketHeader::IntactHeaderSize(true);
	if (len < intactSize)
		return 0;

	uint32_t length = 0;
	memcpy(&length, data, PacketHeader::GetLengthSize());

	if (len < length)
		return 0;

	return length;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		패킷을 디스패치 한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::_onPacketDispatch(Core::NetBuffer& buffer)
{
	if (Config::GetCollectPktBot())
	{
		auto mSec = Core::Time::GetCurTimeMSec();
		auto bufferSize = buffer.getLength();
		_packetFile.write(reinterpret_cast<const char*>(&mSec), sizeof(mSec));
		_packetFile.write(reinterpret_cast<const char*>(&bufferSize), sizeof(bufferSize));
		_packetFile.write(reinterpret_cast<const char*>(buffer.getData()), buffer.getLength());
	}

	PacketHeader pktHeader;
	PacketReader pktReader;
	pktReader.setVersion(_packetVersion);

	if (!pktHeader.generate(buffer.getData(), pktReader, true))
	{
		WARN_LOG("kickout!! header is invalid [userId: %llu]", getId());
		closeSocket();
		return;
	}

	if (_numbering++ != pktHeader.getNumbering())
	{
		WARN_LOG(
			"numbering is different [server: %d, client: %d]",
			_numbering - 1,
			pktHeader.getNumbering());

		WARN_LOG("kickout!! numbering is invalid [userId: %llu]", getId());
		closeSocket();

		return;
	}

	if (pktHeader.getPacketId() <= (int16_t)PacketId::PktPlayerCreateReq)
	{
		_dispather.dispatch(this, pktHeader, pktReader);
	}
	else
	{
		if (pktHeader.getPacketId() == (int16_t)PacketId::PktWorldMoveStartReq)
		{
			_dispather.dispatch(this, pktHeader, pktReader);
		}
		else
		{
			_onPacketDispatchInRoom(pktReader, pktHeader.getPacketId());
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  디스패치한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::_onPacketDispatchInRoom(PacketReader& pktReader, int16_t packetId)
{
	auto origin = _dispather.getPacket(packetId);
	if (!origin)
	{
		WARN_LOG("dispather.getPacket not found:%d", packetId);
		return;
	}

	auto player = getPlayer();
	if (!player || !player->getRoom())
	{
		WARN_LOG("not in room. [packetId: %s(%d)]", origin->getPacketName(), packetId);
		return;
	}

	auto pktBase = origin->clone();
	if (!pktBase->deserialize(pktReader))
	{
		WARN_LOG("Failed to deserialize( %u )", pktBase->getPacketId());
		return;
	}

	//InsertPacketLog(1, getId(), player ? player->getId() : 0, *pktBase);

	int64_t delayTime = 0;

	auto& packetQueue = getPacketQueue(*pktBase);

	while (!packetQueue.push(pktBase))
	{
		if (!delayTime)
		{
			WARN_LOG("push failed");
			delayTime = Core::Time::GetCurTimeMSec();
		}

		if (Core::Time::GetCurTimeMSec() - delayTime > 10000)
		{
			WARN_LOG("packet queue push delayed [userId: %lld, packetId: %d]", getId(), packetId);
			return closeSocket();
		}

		Core::IThread::Sleep(1);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		연결을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::_onConncted()
{
	_lastPingTime = Core::Time::GetCurTimeMSec();
	INFO_LOG("onConnected!!, this: %p, remote: %s", this, getRemoteAddressIpv4().c_str());

	if (Config::GetCollectPktBot())
	{
		std::string fileName = Core::StringUtil::ExtractDriveDir(
			Core::AppInstance::GetBundlePath()) + STRINGUTIL_FORMAT("pkt_%p.bin", this);
		_packetFile.open(fileName, std::ios::out | std::ios::binary);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		연결 종료를 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::_onClosed()
{
	_closeProcess();
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  close때 처리하는 
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::_closeProcess()
{
	if (_isCloseProcessed)
		return;

	_isCloseProcessed = true;

	INFO_LOG("disconnected. [uid: %llu, ip: %s, this: %p]", getId(), getRemoteAddressIpv4().c_str(), this);

	if (auto player = getPlayer())
	{
		auto room = player->getRoom();
		if (!room)
		{
			PlayerManager::Instance().remove(getId());
		}
		else
		{
			room->runTask(Caller, [player](RoomThreadContextRef context)
				{
					player->disconnnectSave(context);
				});
		}
	}

	unPossess();
	UserManager::Instance().remove(getId());

	if (Config::GetCollectPktBot())
	{
		_packetFile.close();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		로그를 기록한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::_dispatchLog(const PktBase& pktBase) const
{
	switch ((PacketId)(pktBase.getPacketId()))
	{
	case PacketId::PktPingReq:
	case PacketId::PktPlayerMoveReq:
		return;
	default:
		break;
	}
	PktObjId playerId = 0;
	if (auto player = getPlayer())
		playerId = player->getId();

	std::string url = STRINGUTIL_FORMAT(
		"%s(%d)?uid=%llu&pid=%lld",
		pktBase.getPacketName(),
		pktBase.getPacketId(),
		_id,
		playerId);

	INFO_LOG(url.c_str());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		유저를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
UserPtr User::Get(uint64_t id)
{
	UserPtr user;
	UserManager::Instance().get(id, user);
	return user;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	킵얼라이브 체크한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void User::checkKeepAlive(int64_t curTime)
{
	if (_lastPingTime && _lastPingTime + (60000 * 10) < curTime)
	{
		WARN_LOG("ping check over user disconnected. [userId: %lld, interval:%lld, %p]", getId(), curTime - _lastPingTime, this);
		_lastPingTime = 0;
		closeSocket();
	}
}
