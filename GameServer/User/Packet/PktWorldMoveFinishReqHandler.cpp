////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktWorldMoveFinishReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktWorldMoveFinishReqHandler.h"

#include <Protocol/Struct/PktPlayer.h>

#include "../User.h"
#include "Actor/ActorTypes.h"
#include "Actor/Player.h"
#include "Room/World.h"
#include "Room/WorldManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktWorldMoveFinishReqHandler::onHandler(User& user, PktWorldMoveFinishReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktWorldMoveFinishReqHandler::_onHandler(User& user, PktWorldMoveFinishReq& req, std::shared_ptr<Ack>& ack)
{
	auto player = user.getPlayer();
	if (!player)
	{
		WARN_LOG("player not exist [uid:%llu]", user.getId());
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	auto room = player->getRoom();
	if (!room)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	user.setCustomNagle(true);

	player->setWorldMoveLoading(false);
	player->updateSight();

	ack->setTargetWorldId(room->getId());

	user.send(*ack);
}


