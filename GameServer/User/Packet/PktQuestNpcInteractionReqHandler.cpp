////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktQuestNpcInteractionReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktQuestNpcInteractionReqHandler.h"

#include <Data/Info/NpcInfo.h>
#include <Data/Info/QuestInfo.h>
#include <Protocol/Struct/PktQuest.h>
#include "Actor/Player.h"
#include "Logic/Quest/QuestComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktQuestNpcInteractionReqHandler::onHandler(User& user, PktQuestNpcInteractionReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktQuestNpcInteractionReqHandler::_onHandler(User& user, PktQuestNpcInteractionReq& req, std::shared_ptr<Ack>& ack)
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

	NpcInfoPtr npcInfo(req.getNpcInfoId());
	if (!npcInfo)
	{
		ack->setResult(EResultCode::InfoNotExist);
		user.send(*ack);
		return;
	}

	auto& questComponent = player->getQuestComponent();

	auto quest = questComponent.findQuest(questInfo->getId());
	if (!quest)
	{
		ack->setResult(EResultCode::QuestAreadyProgress);
		user.send(*ack);
		return;
	}

	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);

	auto updateQuests = std::make_shared<std::vector<PktQuest>>();
	player->broadcastNpcInteraction(*cacheTx, *npcInfo, req.getConditionType());

	cacheTx->ifFailed(*player, [ack, player]()
		{
			ack->setResult(EResultCode::DBError);
			player->send(*ack);
			return;
		});

	cacheTx->ifSucceed(*player, [player, updateQuests, ack]()
		{
			player->send(*ack);

			PktQuestUpdateNotify notify;
			notify.setAddOrUpdateQuests(*updateQuests);
			player->send(notify);
		});

	cacheTx->addBusyUpdater(player->createBusyUpdater());
	cacheTx->run();
}


