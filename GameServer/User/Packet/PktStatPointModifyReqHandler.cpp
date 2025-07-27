////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktStatPointModifyReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktStatPointModifyReqHandler.h"

#include <Protocol/Struct/PktGrowth.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktStatPointModifyReqHandler::onHandler(User& user, PktStatPointModifyReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktStatPointModifyReqHandler::_onHandler(User& user, PktStatPointModifyReq& req, std::shared_ptr<Ack>& ack)
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

	const auto& reqDistribute = req.getStatPointDistribute();
	PktStatPointDistribute curDistribute;

	auto& comp = player->getCharacterComponent();
	comp.exportTo(req.getCharacterId(), curDistribute);

	auto reqPlus =
		reqDistribute.getStr() +
		reqDistribute.getDex() +
		reqDistribute.getIntellect() +
		reqDistribute.getRemain();


	if (reqPlus - reqDistribute.getRemain() > 0)
	{
		if (reqDistribute.getStr() < curDistribute.getStr() ||
			reqDistribute.getDex() < curDistribute.getDex() ||
			reqDistribute.getIntellect() < curDistribute.getIntellect() ||
			reqDistribute.getRemain() > curDistribute.getRemain())
		{
			ack->setResult(EResultCode::ParamError);
			user.send(*ack);
			return;
		}
	}


	auto curPlus =
		curDistribute.getStr() +
		curDistribute.getDex() +
		curDistribute.getIntellect() +
		curDistribute.getRemain();

	if (reqPlus != curPlus)
	{
		ack->setResult(EResultCode::ParamError);
		user.send(*ack);
		return;
	}

	auto cacheTx = std::make_shared<CacheTx>(Caller);
	auto ret = comp.change(*cacheTx, req.getCharacterId(), reqDistribute);
	if (ret != EResultCode::Success)
	{
		ack->setResult(ret);
		user.send(*ack);
		return;
	}

	cacheTx->ifSucceed(*player, [player, req, ack]()
		{
			player->getCharacterComponent().exportTo(req.getCharacterId(), ack->getStatPointDistribute());
			player->send(*ack);

			PktActorStatChangeNotify notify;
			notify.setId(player->getId());

			if (!notify.getStatList().empty())
				player->send(notify);
		});
	cacheTx->ifFailed(*player, [player, ack]()
		{
			ack->setResult(EResultCode::DBError);
			player->send(*ack);
		});

	cacheTx->addBusyUpdater(player->createBusyUpdater());
	cacheTx->run();
}


