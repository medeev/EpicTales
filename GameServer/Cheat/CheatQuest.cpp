

#include "Pch.h"
#include "CheatQuest.h"

#include <Data/Info/QuestInfo.h>
#include <Protocol/Struct/PktQuest.h>
#include <Protocol/Struct/PktRoom.h>

#include "Logic/PlayerManager.h"
#include "Logic/Quest/QuestComponent.h"
#include "User/UserManager.h"
#include "OrmSub/Quest.h"

static CheatQuest s_instanceCheatQuest;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatQuest::CheatQuest()
	:
	Cheat("quest")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatQuest::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.empty())
		return;

	if (params[0] == "start")
	{
		_start(player, params);
		return;
	}
	if (params[0] == "comp")
	{
		_comp(player, params);
		return;
	}
	if (params[0] == "add")
	{
		_add(player, params);
		return;
	}
	if (params[0] == "del")
	{
		_del(player, params);
		return;
	}
	
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트를 시작한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatQuest::_start(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 2)
		return;

	auto infoIdStr = params[1];

	auto infoId = Core::TypeConv::ToInteger(infoIdStr);

	auto& questComponent = player->getQuestComponent();
	auto quest = questComponent.findQuest(infoId);
	if (quest)
	{
		WARN_LOG("quest is aready started.");
		return;
	}

	PktQuestUpdateNotify notify;
	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);

	PktQuest pktQuest;
	if (!questComponent.insertQuest(*cacheTx, infoId, pktQuest))
		return;

	notify.getAddOrUpdateQuests().push_back(pktQuest);

	cacheTx->ifSucceed(*player, [player, notify]()
		{
			player->send(notify);
		});

	cacheTx->run();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  진행퀘스트를 완료가능상태로 변경한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatQuest::_comp(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 3)
		return;

	auto infoIdStr = params[1];
	auto infoId = Core::TypeConv::ToInteger(infoIdStr);
	QuestInfoPtr questInfo(infoId);
	if (!questInfo)
	{
		WARN_LOG("quest is not exist");
		return;
	}

	PktQuestUpdateNotify notify;
	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);

	auto& questComponent = player->getQuestComponent();
	if (!questComponent.completeQuest(*cacheTx, *questInfo, notify.getAddOrUpdateQuests()))
	{
		WARN_LOG("quest is not enable complete. plz start first, and so on");
		return;
	}

	cacheTx->ifSucceed(*player, [player, notify, questInfo]()
		{
			player->send(notify);

			CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
			PktQuestUpdateNotify completeUpdateNotify;
			player->getQuestComponent().onQuestComplete(*cacheTx, *questInfo, completeUpdateNotify.getAddOrUpdateQuests());

			if (!completeUpdateNotify.getAddOrUpdateQuests().empty())
			{
				cacheTx->ifSucceed(*player, [completeUpdateNotify, player]()
					{
						player->send(completeUpdateNotify);
					});

				cacheTx->addBusyUpdater(player->createBusyUpdater());
				cacheTx->run();
			}
		});

	cacheTx->addBusyUpdater(player->createBusyUpdater());
	cacheTx->run();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 완료 퀘스트를 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatQuest::_add(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 2)
		return;

	auto infoIdStr = params[1];

	auto infoId = Core::TypeConv::ToInteger(infoIdStr);

	auto& questComponent = player->getQuestComponent();

	auto quest = questComponent.findQuest(infoId);
	if (!quest)
	{
		WARN_LOG("quest is not started. plz start first");
		return;
	}

	PktQuestUpdateNotify notify;
	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);

	auto cacheQuest = cacheTx->acquireObject(*player, quest);
	cacheQuest->setState((uint8_t)EQuestState::Completed);
	cacheQuest->updateCache();

	cacheTx->ifSucceed(*player, [player, notify]()
		{
			player->send(notify);
		});

	cacheTx->run();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  완료퀘스트를 삭제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatQuest::_del(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 3)
		return;

	auto infoIdStr = params[1];
	auto countStr = params[2];

	PktInfoId infoId = Core::TypeConv::ToInteger(infoIdStr);
	auto count = Core::TypeConv::ToInteger(countStr);

	auto& questComponent = player->getQuestComponent();
	auto quest = questComponent.findQuest(infoId);
	if (!quest)
	{
		WARN_LOG("quest is not started. plz start first");
		return;
	}

	PktQuestUpdateNotify notify;
	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);

	auto cacheQuest = cacheTx->acquireObject(*player, quest);
	cacheQuest->setState((uint8_t)EQuestState::Completable);
	cacheQuest->updateCache();

	cacheTx->ifSucceed(*player, [player, notify]()
		{
			player->send(notify);
		});

	cacheTx->run();
}
