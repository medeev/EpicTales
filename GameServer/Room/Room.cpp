

#include "Pch.h"
#include "Room.h"

#include <Core/Thread/IThread.h>
#include <Core/Thread/ThreadChecker.h>
#include <Core/Util/IdFactory.h>
#include <Protocol/Struct/PktActor.h>
#include <Protocol/Struct/PktPlayer.h>
#include <Protocol/Struct/PktRoom.h>

#include "Actor/Player.h"
#include "Room/World.h"
#include "Room/RoomThreadContext.h"
#include "User/User.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
Room::Room(RoomThread& thread)
	:
	_thread(thread),
	_removable(false),
	_cellManager(*this)
{
	_taskManager = std::make_shared<Core::TaskManager>("Room", thread);
	_id = Core::IdFactory::CreateId();
	_createTime = Core::Time::GetCurTimeMSec();
	_forceUpdateCheckTime = _createTime + 5000;
	_time = _createTime;
	_deltaMsec = 0;
	_deltaSec = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
Room::~Room()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  대상셀이 소스셀의 인접 셀인지 판단한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Room::isNearChellNo(int32_t src, int32_t dest) const
{
	return _cellManager.isNearCellNo(src, dest);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  초기환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::initialize()
{
	initComponents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  해제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::finalize()
{
	finalizeComponents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		액터를 추가한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Room::addActor(ActorPtr actor)
{
	checkThread();

	ENSURE(actor, return false);
	ENSURE(actor->getId(), return false);

	auto world = actor->getWorld();
	if (world)
	{
		WARN_LOG(
			"already entered room. [actorType: %d, actorId: %llu",
			actor->getType(),
			actor->getId());
		return false;
	}

	_cellManager.addActor(*actor);
	if (_actors.find(actor->getId()) != _actors.end())
	{
		WARN_LOG(
			"actor already exist. [id: %llu, type: %d",
			actor->getId(),
			actor->getType());
	}
	_actors[actor->getId()] = actor;

	switch (actor->getType())
	{
	case EActorType::Player:
		{
			if (_players.find(actor->getId()) != _players.end())
			{
				WARN_LOG(
					"player already exist. [id: %llu]", actor->getId());
			}

			PlayerPtr player = std::dynamic_pointer_cast<Player>(actor);
			_players[actor->getId()] = player;
			_playerCount.fetch_add(1);

			if (UserPtr user = player->getUser())
				user->leaveWaitRoom();

			
		}
		break;
	case EActorType::Npc:
		{
			if (_npcs.find(actor->getId()) != _npcs.end())
				WARN_LOG(
					"npc already exist. [id: %lld]",
					actor->getId());
			_npcs[actor->getId()] = std::dynamic_pointer_cast<Npc>(actor);
		}
		break;
	}

	actor->setRoom(this);
	actor->beginPlay();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		액터를 제거될때
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::_removeActor(Actor* actor)
{
	ENSURE(actor, return);
	ENSURE(actor->getId(), return);

	PktActorDestroyNotify notify;
	notify.getActorIds().push_back(actor->getId());
	sendToNearExcept(*actor, notify);

	switch (actor->getType())
	{
	case EActorType::Player:
		_players.erase(actor->getId());
		_playerCount.fetch_sub(1);
		break;
	case EActorType::Npc:
		_npcs.erase(actor->getId());
		break;
	}

	
	_cellManager.foreachPlayer(
		actor->getCellNo(),
		[actor](PlayerPtr player)
		{
			player->removeSeeingActorId(actor->getId());
		});
	_cellManager.removeActor(actor);

	// 1. 해제 후 떠난상태에서 LeaveCallback
	actor->endPlay();

	// 2. 룸 해제
	actor->setRoom(nullptr);

	// 3. 해제 후 떠난상태에서 LeaveCallback
	actor->endPlayPost();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		갱신한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::update()
{
	if (_taskManager)
		_taskManager->update(_time);
	updateComponent(_time);

	if (_playerCount.load() || _time < _forceUpdateCheckTime)
	{
		for (auto it = _actors.begin(); it != _actors.end(); )
		{
			auto& actor = it->second;
			actor->update(_time);

			if (actor->isRoomRemovable())
			{
				actor->setRoomRemovable(false);
				auto actor = it->second;
				//INFO_LOG("actor list erase [actorid:%llu, roomId:%llu]", actor->getId(), getId());
				it = _actors.erase(it);

				_removeActor(actor.get());
			}
			else
				++it;
		}

		for (auto& pair : _players)
		{
			const auto& player = pair.second;
			ENSURE(player, continue);

			if (UserPtr user = player->getUser())
				user->sendNaglePacket();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	시야를 업데이트한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::_updateSight(Actor& actor, const CellNos* cellNos, bool isSendedSpawnNoti)
{
	checkThread();

	ENSURE(actor.getRoom() == this, return);

	PktSightEnterNotify pktSightEnterNotifyToTarget;
	PktSightLeaveNotify pktSightLeaveNotifyToTarget;

	actor.exportTo(pktSightEnterNotifyToTarget);
	pktSightLeaveNotifyToTarget.getActorIds().emplace_back(actor.getId());

	if (actor.getType() == EActorType::Player)
	{
		auto player = static_cast<Player*>(&actor);

		PktSightEnterNotify pktSightEnterNotify;
		PktSightLeaveNotify pktSightLeaveNotify;
		bool someoneSightEntered = false;

		auto func = [this, player, &pktSightEnterNotify, &pktSightEnterNotifyToTarget, &pktSightLeaveNotify, &pktSightLeaveNotifyToTarget, &someoneSightEntered](const ActorPtr& eachActor)
			{
				if (player == eachActor.get())
					return;

				if (eachActor->getType() == EActorType::Player)
				{
					auto eachPlayer = std::dynamic_pointer_cast<Player>(eachActor);
					ENSURE(eachPlayer, return);

					if (eachPlayer->canSee(*player))
					{
						if (!eachPlayer->isSeeingActorId(player->getId()))
						{
							eachPlayer->addSeeingActorId(player->getId());
							if (auto eachUser = eachPlayer->getUser())
								eachUser->send(pktSightEnterNotifyToTarget);
						}
					}
					else
					{
						if (eachPlayer->isSeeingActorId(player->getId()))
						{
							eachPlayer->removeSeeingActorId(player->getId());
							eachPlayer->send(pktSightLeaveNotifyToTarget);
						}
					}
				}

				if (!player->canSee(*eachActor))
				{
					if (player->isSeeingActorId(eachActor->getId()))
					{
						player->removeSeeingActorId(eachActor->getId());
						pktSightLeaveNotify.getActorIds().emplace_back(eachActor->getId());
					}
				}
				else
				{
					if (!player->isSeeingActorId(eachActor->getId()))
					{
						eachActor->exportTo(pktSightEnterNotify);
						player->addSeeingActorId(eachActor->getId());
						someoneSightEntered = true;
					}
				}
			};

		if (cellNos)
		{
			_cellManager.foreachActor(*cellNos, func);
		}
		else
		{
			_cellManager.foreachActor(actor.getCellNo(), func);
		}

		if (someoneSightEntered)
			player->send(pktSightEnterNotify);

		if (!pktSightLeaveNotify.getActorIds().empty())
			player->send(pktSightLeaveNotify);
	}
	else
	{
		auto func = [&actor, &pktSightEnterNotifyToTarget, &pktSightLeaveNotifyToTarget, isSendedSpawnNoti](const PlayerPtr& eachPlayer)
			{

				if (eachPlayer.get() == &actor)
					return;

				if (eachPlayer->canSee(actor))
				{
					if (!eachPlayer->isSeeingActorId(actor.getId()))
					{
						eachPlayer->addSeeingActorId(actor.getId());

						if (!isSendedSpawnNoti)
						{
							if (auto eachUser = eachPlayer->getUser())
								eachUser->send(pktSightEnterNotifyToTarget);
						}
					}
				}
				else
				{
					if (eachPlayer->isSeeingActorId(actor.getId()))
					{
						eachPlayer->removeSeeingActorId(actor.getId());
						if (auto eachUser = eachPlayer->getUser())
							eachUser->send(pktSightLeaveNotifyToTarget);
					}
				}
			};

		if (cellNos)
		{
			_cellManager.foreachPlayer(*cellNos, func);
		}
		else
		{
			_cellManager.foreachPlayer(actor.getCellNo(), func);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	모든 플레이어들에게 패킷을 전송한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::sendToAll(PktBase& packet) const
{
	checkThread();

	for (const auto& playerPair : _players)
	{
		auto player = playerPair.second;
		player->send(packet);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	대상 액터를 제외한 모든 플레이어들에게 패킷을 전송한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::sendToAllExcept(const Actor& actor, PktBase& packet) const
{
	checkThread();

	for (const auto& playerPair : _players)
	{
		auto player = playerPair.second;
		if (player.get() != &actor)
			player->send(packet);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	제외대상을 체크하여 플레이어들에게 패킷을 전송한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::sendToAllExcept(PktBase& packet, const SendExceptCallback& exceptfunc) const
{
	checkThread();

	for (const auto& playerPair : _players)
	{
		auto player = playerPair.second;

		if (exceptfunc)
		{
			if (!exceptfunc(player))
				player->send(packet);
		}
		else
		{
			player->send(packet);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	대상 액터를 포함한 근처 플레이어들에게 패킷을 전송한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::sendToNear(const Actor& actor, PktBase& packet) const
{
	checkThread();

	_cellManager.foreachPlayer(
		actor.getCellNo(),
		[&actor, &packet](const PlayerPtr& eachPlayer)
		{
			auto user = eachPlayer->getUser();
			if (!user)
				return;

			if (!eachPlayer->canSee(actor))
				return;

			user->send(packet);
		});
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	대상 액터를 포함한 근처 플레이어들에게 패킷을 전송한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::sendToNearExcept(const Actor& actor, PktBase& packet) const
{
	checkThread();

	_cellManager.foreachPlayer(
		actor.getCellNo(),
		[&actor, &packet](const PlayerPtr& eachPlayer)
		{
			if (actor.getId() == eachPlayer->getId())
				return;

			auto user = eachPlayer->getUser();
			if (!user)
				return;

			if (!eachPlayer->canSee(actor))
				return;

			user->send(packet);
		});
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스레드가 올바른지 체크한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Room::checkThread() const
{
	if (_thread.isRunning())
	{
		return Core::ThreadChecker::Check(
			_thread.getId(),
			"Room, id: %llu",
			_id);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		작업관리자를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
Core::TaskManagerPtr Room::getTaskManager()
{
	return _taskManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		액터를 찾는다
////////////////////////////////////////////////////////////////////////////////////////////////////
const ActorPtr Room::findActor(PktObjId objId) const
{
	checkThread();

	auto iter = _actors.find(objId);
	if (iter != _actors.end())
		return iter->second;

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		플레이어를 순회한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::forEachPlayer(const PlayerCallback& callback) const
{
	checkThread();
	for (const auto& pair : _players)
		callback(pair.second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	셀 액터를 순회한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::forEachActor(int32_t cellNo, const ActorCallback& callback) const
{
	checkThread();
	_cellManager.foreachActor(cellNo, callback);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	셀 액터를 순회한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::forEachPlayer(int32_t cellNo, const PlayerCallback& callback) const
{
	checkThread();
	_cellManager.foreachPlayer(cellNo, callback);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	셀 Npc 맵애 대해서 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::forEachNpc(int32_t cellNo, const NpcCallback& callback) const
{
	checkThread();
	_cellManager.foreachNpc(cellNo, callback);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		플레이어를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
const PlayerPtr Room::getPlayer(PktObjId playerId) const
{
	checkThread();

	auto it = _players.find(playerId);
	if (it == _players.end())
		return nullptr;

	return it->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  Npc를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
const NpcPtr Room::getNpc(PktObjId npcId) const
{
	checkThread();

	auto it = _npcs.find(npcId);
	if (it == _npcs.end())
		return nullptr;

	return it->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	룸에서 작업을 한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::runTask(const Core::TaskCaller& taskCaller, const Task& task)
{
	if (_taskManager)
	{
		_taskManager->runTaskWithContext<RoomThreadContext>(taskCaller, task);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  룸에서 작업한다.(동일스레드면 바로시작한다)
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::dispatchTask(const Core::TaskCaller& taskCaller, const Task& task)
{
	if (_taskManager)
	{
		_taskManager->diapatchTaskWithContext<RoomThreadContext>(taskCaller, task);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  출판한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::setPublish(const std::string& channelName, const std::string& message)
{
	auto iter = _subscrives.find(channelName);
	if (iter == _subscrives.end())
	{
		WARN_LOG("channelName not exist [name:%s]", channelName.c_str());
		return;
	}

	auto& subscriveActors = iter->second;
	for (auto actorWeak : subscriveActors | std::views::values)
		if (auto actor = actorWeak.lock())
			actor->reciveChannelMessage(channelName, message);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  구독한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::setSubscribe(Actor* actor, const std::string& channelName)
{
	ENSURE(actor, return);

	auto ret = _subscrives[channelName].insert(std::make_pair(actor->getId(), actor->shared_from_this()));
	if (!ret.second)
		WARN_LOG("aready subscrive");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  채널구독을 취소한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::cancelSubscrive(Actor* actor, const std::string& channelName)
{
	ENSURE(actor, return);

	auto iter = _subscrives.find(channelName);
	if (iter == _subscrives.end())
	{
		WARN_LOG("channelName not exist [name:%s]", channelName.c_str());
		return;
	}

	auto& subscriveActors = iter->second;
	subscriveActors.erase(actor->getId());
	if (subscriveActors.empty())
	{
		_subscrives.erase(iter);
	}

}
