////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktQuestCancelReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktQuestCancelReqHandler.h"

#include <Data/Info/QuestInfo.h>
#include <Protocol/Struct/PktQuest.h>

#include "Logic/Quest/QuestComponent.h"
#include "OrmSub/Quest.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktQuestCancelReqHandler::onHandler(User& user, PktQuestCancelReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktQuestCancelReqHandler::_onHandler(User& user, PktQuestCancelReq& req, std::shared_ptr<Ack>& ack)
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

	QuestInfoPtr questInfo(req.getQuestInfoId());
	if (!questInfo)
	{
		ack->setResult(EResultCode::InfoNotExist);
		user.send(*ack);
		return;
	}

	auto& questComponent = player->getQuestComponent();

	auto quest = questComponent.findQuest(questInfo->getId());
	if (!quest)
	{
		ack->setResult(EResultCode::QuestNotExist);
		user.send(*ack);
		return;
	}

	auto questCur = questComponent.findQuest(questInfo->getId());
	if (!questCur)
	{
		ack->setResult(EResultCode::QuestNotProgress);
		user.send(*ack);
		return;
	}

	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	auto cacheQuestCur = cacheTx->acquireObject(*player, questCur);
	cacheQuestCur->delCache();

	cacheTx->ifFailed(*player, [ack, player]()
		{
			ack->setResult(EResultCode::DBError);
			player->send(*ack);
		});

	cacheTx->ifSucceed(*player, [ack, player]()
		{
			player->send(*ack);
		});

	cacheTx->addBusyUpdater(player->createBusyUpdater());
	cacheTx->run();
}


