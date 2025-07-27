

#include "Pch.h"
#include "Quest.h"

#include <Data/Info/QuestInfo.h>
#include <Protocol/Struct/PktQuest.h>

#include "Actor/Player.h"
#include "Logic/PlayerManager.h"
#include "Logic/Quest/QuestComponent.h"

void Quest::applyCache(DBOrm::OrmObjectPtr origin) const
{
	switch (getCrudState())
	{
	case DBOrm::OrmObject::CRUD::Insert:
		{
			if (auto quest = std::dynamic_pointer_cast<Quest>(origin))
			{
				PlayerPtr player;
				if (PlayerManager::Instance().get(_ownerId, player))
					player->getQuestComponent().insertQuest(quest);
			}
		}
		break;

	case DBOrm::OrmObject::CRUD::Update:
		{
			if (auto quest = std::dynamic_pointer_cast<Quest>(origin))
			{
				if ((EQuestState)quest->getState() == EQuestState::Completable &&
					(EQuestState)getState() == EQuestState::Completed)
				{
					PlayerPtr player;
					if (PlayerManager::Instance().get(_ownerId, player))
					{
						player->getQuestComponent().deleteQuest(quest->getInfoId());
						player->getQuestComponent().insertCompletedQuest(quest);
					}
				}
			}
		}
		break;

	case DBOrm::OrmObject::CRUD::Delete:
		{
			PlayerPtr player;
			if (PlayerManager::Instance().get(_ownerId, player))
				player->getQuestComponent().deleteQuest(getInfoId());
		}
		break;

	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  클론한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Quest> Quest::clone() const
{
	auto clone = std::make_shared<Quest>(_questInfo);
	copyMember(*clone);

	return clone;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트 정보를 내보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Quest::exportTo(PktQuest& pktQuest) const
{
	pktQuest.setQuestInfoId(_questInfo.getId());
	pktQuest.setQuestState((EQuestState)_state);
	pktQuest.getTaskCounts().clear();
	pktQuest.getTaskCounts().push_back(getTaskCount0());
	pktQuest.getTaskCounts().push_back(getTaskCount1());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  테스크 횟수를 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Quest::setTaskCount(int32_t index, int32_t count)
{
	if (index >= 2 || index < 0)
	{
		WARN_LOG("invalid quest task index");
		return;
	}

	if (index == 0)
		setTaskCount0(count);
	else if (index == 1)
		setTaskCount1(count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  테스크 횟수를 가져온다.
////////////////////////////////////////////////////////////////////////////////////////////////////
int32_t Quest::getTaskCount(int32_t index) const
{
	if (index >= 2 || index < 0)
	{
		WARN_LOG("invalid quest task index");
		return 0;
	}

	if (index == 0)
		return getTaskCount0();

	return getTaskCount1();
}
