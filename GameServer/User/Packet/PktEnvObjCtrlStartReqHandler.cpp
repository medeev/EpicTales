////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktEnvObjCtrlStartReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktEnvObjCtrlStartReqHandler.h"

#include <Data/Info/EnvObjInfo.h>

#include "Actor/EnvObj.h"
#include "Logic/Fsm/FsmComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktEnvObjCtrlStartReqHandler::onHandler(User& user, PktEnvObjCtrlStartReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktEnvObjCtrlStartReqHandler::_onHandler(User& user, PktEnvObjCtrlStartReq& req, std::shared_ptr<Ack>& ack)
{
	auto player = user.getPlayer();
	if (!player)
	{
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

	auto envObj = room->findActor<EnvObj>(req.getEnvObjId());
	if (!envObj)
	{
		ack->setResult(EResultCode::InvalidActorId);
		user.send(*ack);
		return;
	}


	if (player->getState() != EFsmStateType::PlayerStateIdle)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	float distSqr = (float)envObj->getInfo().getInteractionDist() * (float)envObj->getInfo().getInteractionDist();

	if (envObj->getLocation().getSquaredDistance(player->getLocation()) > distSqr)
	{
		ack->setResult(EResultCode::InvalidInteractionDist);
		user.send(*ack);
		return;
	}

	if (!envObj->isUnLimitedCtrl())
		if (envObj->getRemainCtrlCount() == 0)
		{
			ack->setResult(EResultCode::NotEnoughtCount);
			user.send(*ack);
			return;
		}

	player->setCtrledEnvObj(envObj);
	player->getFsmComponent().changeState(EFsmStateType::PlayerStateCtrledEnvObj);

	ack->setEnvObjId(envObj->getId());
	player->send(*ack);
}


