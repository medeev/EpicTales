////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktQuestCompleteReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktQuestCompleteReqHandler.h"

#include <Data/Info/QuestInfo.h>
#include "Logic/Quest/QuestComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktQuestCompleteReqHandler::onHandler(User& user, PktQuestCompleteReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktQuestCompleteReqHandler::_onHandler(User& user, PktQuestCompleteReq& req, std::shared_ptr<Ack>& ack)
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

	auto questInfoId = req.getQuestInfoId();
	QuestInfoPtr questInfo(questInfoId);
	if (!questInfo)
	{
		ack->setResult(EResultCode::InfoNotExist);
		user.send(*ack);
		return;
	}

	auto& questComponent = player->getQuestComponent();
	if (!questComponent.isCompleteEnable(*questInfo))
	{
		ack->setResult(EResultCode::QuestNotClearCondition);
		user.send(*ack);
		return;
	}

	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);

	PktQuestUpdateNotify notify;
	if (!questComponent.completeQuest(*cacheTx, *questInfo, notify.getAddOrUpdateQuests()))
	{
		ack->setResult(EResultCode::QuestNotCompletable);
		user.send(*ack);
		return;
	}

	cacheTx->ifFailed(*player, [ack, player]()
		{
			ack->setResult(EResultCode::DBError);
			player->send(*ack);
		});

	cacheTx->ifSucceed(*player, [player, notify, questInfo, ack]()
		{
			player->send(*ack);
			player->send(notify);

			{
				PktQuestUpdateNotify pktQuestUpdateNotify;
				CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
				player->getQuestComponent().onQuestComplete(*cacheTx, *questInfo, pktQuestUpdateNotify.getAddOrUpdateQuests());

				if (!pktQuestUpdateNotify.getAddOrUpdateQuests().empty())
				{
					cacheTx->ifSucceed(*player, [player, pktQuestUpdateNotify]()
						{
							player->send(pktQuestUpdateNotify);
						});

					cacheTx->addBusyUpdater(player->createBusyUpdater());
					cacheTx->run();
				}
			}

		});

	cacheTx->addBusyUpdater(player->createBusyUpdater());
	cacheTx->run();
}


