////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktQuestStartReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktQuestStartReqHandler.h"

#include <Data/Info/QuestInfo.h>
#include <Protocol/Struct/PktQuest.h>

#include "Logic/Quest/QuestComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktQuestStartReqHandler::onHandler(User& user, PktQuestStartReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktQuestStartReqHandler::_onHandler(User& user, PktQuestStartReq& req, std::shared_ptr<Ack>& ack)
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
	if (!questComponent.isStartEnable(*questInfo))
	{
		ack->setResult(EResultCode::QuestNotStartCondition);
		user.send(*ack);
		return;
	}

	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);

	PktQuest pktQuest;
	if (!questComponent.startQuest(*cacheTx, questInfo->getId(), pktQuest))
	{
		ack->setResult(EResultCode::LogicError);
		user.send(*ack);
		return;
	}

	cacheTx->ifFailed(*player, [player, ack]()
		{
			ack->setResult(EResultCode::DBError);
			player->send(*ack);
			return;
		});

	cacheTx->ifSucceed(*player, [player, pktQuest, ack]()
		{
			player->send(*ack);

			PktQuestUpdateNotify notify;
			notify.getAddOrUpdateQuests().push_back(pktQuest);

			player->send(notify);
		});

	cacheTx->addBusyUpdater(player->createBusyUpdater());
	cacheTx->run();
}


