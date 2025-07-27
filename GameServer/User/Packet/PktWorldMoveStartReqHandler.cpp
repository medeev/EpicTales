////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktWorldMoveStartReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktWorldMoveStartReqHandler.h"

#include <Data/Info/WorldInfo.h>
#include <Data/WorldEntityData/WorldEntityData.h>
#include <Protocol/Struct/PktPlayer.h>
#include <Protocol/Struct/PktRoom.h>

#include "../User.h"
#include "Actor/ActorTypes.h"
#include "Actor/Player.h"
#include "Room/WorldManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktWorldMoveStartReqHandler::onHandler(User& user, PktWorldMoveStartReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktWorldMoveStartReqHandler::_onHandler(User& user, PktWorldMoveStartReq& req, std::shared_ptr<Ack>& ack)
{
	auto player = user.getPlayer();
	if (!player)
	{
		WARN_LOG("player is not exist [uid:%llu]", user.getId());
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	if (player->isWorldMoveLoading())
	{
		WARN_LOG("current is loading [pid:%llu]", player->getId());
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}


	WorldInfoPtr worldInfo(req.getWorldInfoId());
	if (!worldInfo)
	{
		WARN_LOG("world info is invalid [pid:%llu, infoId: %u]", player->getId(), req.getWorldInfoId());
		ack->setResult(EResultCode::InfoNotExist);
		user.send(*ack);
		return;
	}

	// 특정정해진 월드Id로 진입
	auto curWorld = player->getWorld();
	WorldPtr nextWorld = WorldManager::Instance().acquireWorld(worldInfo->getId(), false, [worldInfo](RoomThread& thd)
		{
			WorldPtr world = std::make_shared<World>(thd, *worldInfo);
			return world;
		});

	if (!nextWorld)
	{
		WARN_LOG("next world is not exist [pid:%llu, infoId:%u]",
			player->getId(), worldInfo->getId());

		ack->setResult(EResultCode::ParamError);
		user.send(*ack);
		return;
	}

	auto location = player->getLocation();

	if (curWorld == nullptr || (nextWorld->getInfo().getIsVillage()))
	{
		location = player->getDBLocation();
	}


	if (curWorld && curWorld != nextWorld.get())
	{
		if (curWorld->getInfo().getIsVillage())
		{
			if (auto playerStart = curWorld->getInfo().getPlayerStartNearestLocation(location))
			{
				location = playerStart->getConnectedPlayerStartLocation();
				if (nextWorld->getInfo().getId() != playerStart->getConnectedWorldInfoId())
				{
					WARN_LOG("current near playerStart's connectedWorld not same [req:%u, conn:%u]",
						nextWorld->getInfo().getId(), playerStart->getConnectedWorldInfoId());

					ack->setResult(EResultCode::ParamError);
					user.send(*ack);
					return;
				}
			}
		}
	}

	World::DoWorldMoveStart(player, nextWorld, location, [player, ack]()
		{
			player->send(*ack);
		});
}


