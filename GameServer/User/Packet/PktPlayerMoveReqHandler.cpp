////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPlayerMoveReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktPlayerMoveReqHandler.h"

#include <Data/Info/WorldInfo.h>
#include <Protocol/Struct/PktPlayer.h>

#include "../User.h"
#include "Actor/Player.h"
#include "Room/Room.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPlayerMoveReqHandler::onHandler(User& user, PktPlayerMoveReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPlayerMoveReqHandler::_onHandler(User& user, PktPlayerMoveReq& req, std::shared_ptr<Ack>& ack)
{
	auto player = user.getPlayer();
	if (!player)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	if (player->isWorldMoveLoading())
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	auto world = player->getWorld();
	if (!world)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	if (!world->getInfo().getIsVillage())
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	player->setLocation(req.getLocation());
	player->setDirection(Vector::FromYaw(req.getYaw()));

	PktPlayerMoveNotify notify;
	notify.setPlayerId(player->getId());
	notify.setLocation(req.getLocation());
	world->sendToNearExcept(*player, notify);

	user.send(*ack);
}


