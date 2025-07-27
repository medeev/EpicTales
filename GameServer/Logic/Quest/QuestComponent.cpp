

#include "Pch.h"
#include "QuestComponent.h"

#include <Data/IfConditionData/IfConditionDataBase.h>
#include <Data/IfConditionData/IfConditionDataInteractionNpc.h>
#include <Data/IfConditionData/IfConditionDataItemAcquire.h>
#include <Data/IfConditionData/IfConditionDataItemEquip.h>
#include <Data/IfConditionData/IfConditionDataKillMonster.h>
#include <Data/IfConditionData/IfConditionDataTriggerEnter.h>
#include <Data/Info/CharacterInfo.h>
#include <Data/Info/ItemInfo.h>
#include <Data/Info/NpcInfo.h>
#include <Data/Info/QuestInfo.h>
#include <Protocol/Struct/PktItem.h>
#include <Protocol/Struct/PktQuest.h>

#include "Actor/ActorFactory.h"
#include "Actor/Npc.h"
#include "Actor/Player.h"
#include "Logic/Reward/RewardProcess.h"
#include "OrmSub/Item.h"
#include "OrmSub/Quest.h"
#include "Room/World.h"
#include "../Item/ItemManager.h"
#include "../Item/InventoryComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
QuestComponent::QuestComponent(Player& player)
	:
	super(EComponentType::Quest, player),
	_player(player), _init(false)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
QuestComponent::~QuestComponent()
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  진행중 퀘스트를 내보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::exportTo(std::vector<PktQuest>& dst) const
{
	for (const auto& quest : _quests | std::views::values)
	{
		PktQuest pktQuest;
		pktQuest.setQuestInfoId(quest->getInfoId());
		pktQuest.setQuestState((EQuestState)quest->getState());
		pktQuest.getTaskCounts().push_back(quest->getTaskCount0());
		pktQuest.getTaskCounts().push_back(quest->getTaskCount1());
		dst.emplace_back(std::move(pktQuest));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::initialize()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		정리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::finalize()
{
	_quests.clear();
	_questCompletes.clear();
	_checkIfs.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  룸진입후 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::beginPlay()
{
	if (!_init)
	{
		_init = true;

		_validateDelegate();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	DB객체로 초기화한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool QuestComponent::initializeDB(const DataBaseTarget& db)
{
	auto ormQuests = DBOrm::Quest::SelectListByOwnerId(
		db, _player.getId());

	for (auto questOrm : ormQuests)
	{
		auto infoId = questOrm->getInfoId();
		QuestInfoPtr questInfo(infoId);
		if (!questInfo)
		{
			WARN_LOG("none questInfoId exist. [infoId: %u, ownerId:%llu",
				questOrm->getInfoId(), questOrm->getOwnerId());

			continue;
		}

		auto quest = std::make_shared<Quest>(*questInfo);
		questOrm->copyMember(*quest);

		if ((EQuestState)quest->getState() == EQuestState::Completed)
			_questCompletes.insert(std::make_pair(quest->getInfoId(), quest));
		else
			_quests.insert(std::make_pair(quest->getInfoId(), quest));
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  DB처리 이후에 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::initializeDBPost(const DataBaseTarget& db)
{
	std::vector<PktQuest> pktAddOrUpdate;
	_validateEnableQuest(db, pktAddOrUpdate);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트를 찾는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
const QuestPtr QuestComponent::findQuest(PktInfoId infoId) const
{
	auto iter = _quests.find(infoId);
	if (iter == _quests.end())
		return nullptr;

	return iter->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트를 찾는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
QuestPtr QuestComponent::findQuest(PktInfoId infoId)
{
	return std::const_pointer_cast<Quest>(static_cast<const QuestComponent&>(*this).findQuest(infoId));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  완료 퀘스트를 찾는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
const QuestPtr QuestComponent::findCompletedQuest(PktInfoId infoId) const
{
	auto iter = _questCompletes.find(infoId);
	if (iter == _questCompletes.end())
		return nullptr;

	return iter->second;
}

QuestPtr QuestComponent::findCompletedQuest(PktInfoId infoId)
{
	return std::const_pointer_cast<Quest>(static_cast<const QuestComponent&>(*this).findCompletedQuest(infoId));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  진행중 퀘스트를 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::insertQuest(QuestPtr quest)
{
	ENSURE(quest, return);

	_quests.insert(std::make_pair(quest->getInfoId(), quest));

	_validateDelegate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트를 시작한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool QuestComponent::startQuest(CacheTx& cacheTx, PktInfoId infoId, PktQuest& pktQuest) const
{
	QuestInfoPtr questInfo(infoId);
	if (!questInfo)
	{
		WARN_LOG("not exist questInfo [pid:%llu, infoId:%u", _player.getId(), infoId);
		return false;
	}

	auto quest = findQuest(infoId);
	if (!quest)
	{
		WARN_LOG("not exist quest [pid:%llu, infoId:%u", _player.getId(), infoId);
		return false;
	}

	auto state = (questInfo->getClearConditionDatas().empty()
		? EQuestState::Completable
		: EQuestState::Progressing);

	auto cacheQuest = cacheTx.acquireObject(_player, quest);
	cacheQuest->setState((int8_t)state);
	cacheQuest->setStartTime(Core::Time::GetCurTime());
	cacheQuest->updateCache();

	cacheQuest->exportTo(pktQuest);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  진행퀘스트를 삭제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::deleteQuest(PktInfoId infoId)
{
	_quests.erase(infoId);

	_validateDelegate();

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  완료퀘스트에 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::insertCompletedQuest(QuestPtr quest)
{
	if (!quest)
		return;

	_questCompletes.insert(std::make_pair(quest->getInfoId(), quest));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트를 DB에 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool QuestComponent::insertQuest(
	CacheTx& cacheTx, PktInfoId infoId, PktQuest& pktQuest) const
{
	QuestInfoPtr questInfo(infoId);
	if (!questInfo)
	{
		WARN_LOG("not exist questInfo [pid:%llu, infoId:%u", _player.getId(), infoId);
		return false;
	}

	auto state = (questInfo->getClearConditionDatas().empty()
		? EQuestState::Completable
		: EQuestState::Progressing);

	auto newQuest = std::make_shared<Quest>(*questInfo);
	newQuest->setOwnerId(_player.getId());
	newQuest->setInfoId(questInfo->getId());
	newQuest->setState((int8_t)state);

	newQuest->setStartTime(Core::Time::GetCurTime());
	auto cacheItem = cacheTx.acquireObject(_player, newQuest);
	cacheItem->insertCache();

	newQuest->exportTo(pktQuest);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트를 완료시킨다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool QuestComponent::completeQuest(
	CacheTx& cacheTx, const QuestInfo& questInfo, std::vector<PktQuest>& pktAddOrUpdate) const
{
	auto quest = findQuest(questInfo.getId());
	if (!quest)
	{
		WARN_LOG("not find quest [pid:%llu, infoId:%u", _player.getId(), questInfo.getId());
		return false;
	}

	if ((EQuestState)quest->getState() != EQuestState::Completable)
	{
		WARN_LOG("not find quest [pid:%llu, infoId:%u", _player.getId(), questInfo.getId());
		return false;
	}

	auto cacheQuest = cacheTx.acquireObject(_player, quest);
	cacheQuest->setState((uint8_t)EQuestState::Completed);
	cacheQuest->updateCache();

	if (auto rewardGroup = questInfo.getRewardInfoGroup())
	{
		auto self = _player.shared_from_this<Player>();
		RewardProcess::Process(cacheTx, self, self, rewardGroup);
	}

	PktQuest pktQuest;
	cacheQuest->exportTo(pktQuest);
	pktAddOrUpdate.emplace_back(std::move(pktQuest));

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  테스크를 업데이트 할때 호출된다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool QuestComponent::_onTaskUpdate(
	CacheTx& cacheTx,
	EIfCondition conditionType,
	ConditionCallback updateChecker,
	CompleteCallback completeChecker,
	std::vector<PktQuest>& updateQuests) const
{
	auto playerPtr = _player.shared_from_this<Player>();
	for (const auto& quest : _quests | std::views::values)
	{
		if ((EQuestState)quest->getState() != EQuestState::Progressing)
			continue;

		const auto& conditionDatas = quest->getInfo().getClearConditionDatas();

		bool isModify = false;
		bool isComplete = true;
		for (int index = 0; index < conditionDatas.size(); ++index)
		{
			const auto& conditionData = conditionDatas[index];

			if (conditionData->getType() != conditionType)
				continue;

			if (!updateChecker)
				continue;

			if (!updateChecker(conditionData))
				continue;

			auto cacheQuest = cacheTx.acquireObject(_player, quest);
			auto taskCount = cacheQuest->getTaskCount(index) + 1;
			cacheQuest->setTaskCount(index, taskCount);
			cacheQuest->updateCache();
			isModify = true;

			if (isComplete)
				if (completeChecker)
					if (!completeChecker(cacheTx, conditionData, taskCount))
						isComplete = false;
		}

		if (!isModify)
			continue;

		auto cacheQuest = cacheTx.acquireObject(_player, quest);
		if (isComplete)
			cacheQuest->setState((uint8_t)EQuestState::Completable);

		PktQuest pktQuest;
		cacheQuest->exportTo(pktQuest);
		updateQuests.emplace_back(std::move(pktQuest));
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  모스터 킬시 호출된다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::_onKillMonster(
	CacheTx& cacheTx, PktInfoId npcInfoId) const
{
	auto updateChecker = [npcInfoId](const IfConditionDataBasePtr conditionData)
		{
			auto data = std::dynamic_pointer_cast<IfConditionDataKillMonster>(conditionData);
			if (!data)
				return false;

			if (data->getNpcInfoId() != npcInfoId)
				return false;

			return true;
		};

	CompleteCallback completeChecker = [npcInfoId](CacheTx& cacheTx, const IfConditionDataBasePtr conditionData, int32_t taskCount)
		{
			auto data = std::dynamic_pointer_cast<IfConditionDataKillMonster>(conditionData);
			if (!data)
				return false;

			if (data->getNpcInfoId() != npcInfoId)
				return false;

			return data->getCount() <= taskCount;
		};
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc와 상호작용시 호출된다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::_onInteractionNpc(
	CacheTx& cacheTx,
	const NpcInfo& info,
	EIfCondition conditionType) const
{
	auto checker = [this, &info, conditionType](const IfConditionDataBasePtr conditionData)
		{
			auto data = std::dynamic_pointer_cast<IfConditionDataInteractionNpc>(conditionData);
			if (!data)
				return false;

			if (data->getNpcInfoId() != info.getId())
				return false;

			switch (conditionType)
			{
			case EIfCondition::NpcInteractionByDelivery:
				{
					auto delivery = std::dynamic_pointer_cast<IfConditionDataInteractionNpcByDelivery>(conditionData);
					ENSURE(delivery, return false);
					auto itemInfo = delivery->getItemInfo();
					ENSURE(itemInfo, return false);

					std::vector<ItemPtr> findItems;
					if (!_player.getInvenComponent().findItems(*itemInfo, findItems))
						return false;

					int32_t invenCount = 0;
					for (auto item : findItems) {
						invenCount += item->getAccum();
					}
					return delivery->getCount() <= invenCount;
				}

			default:
				break;
			}
			return false;
		};

	auto player = _player.shared_from_this<Player>();
	CompleteCallback completeChecker = [conditionType, player, this](CacheTx& cacheTx, const IfConditionDataBasePtr conditionData, int32_t taskCount)
		{
			switch (conditionType)
			{
			case EIfCondition::NpcInteractionByDelivery:
				{
					auto delivery = std::dynamic_pointer_cast<IfConditionDataInteractionNpcByDelivery>(conditionData);
					ENSURE(delivery, return false);
					auto itemInfo = delivery->getItemInfo();
					ENSURE(itemInfo, return false);

					PktItemChangeNotify notify;
					if (!player->getInvenComponent().deleteItem(cacheTx, (PktInfoId)itemInfo->getId(), delivery->getCount(), &notify.getChangedItemData()))
						return false;

					cacheTx.ifSucceed(*player, [player, notify]()
						{
							player->send(notify);
						});

					return true;

				}
				break;

			default:
				break;
			}
			return true;
		};

	auto updateQuests = std::make_shared<std::vector<PktQuest>>();
	if (_onTaskUpdate(cacheTx, conditionType, checker, completeChecker, *updateQuests))
	{
		if (!updateQuests->empty())
		{
			auto player = _player.shared_from_this<Player>();
			cacheTx.ifSucceed(_player, [player, updateQuests]()
				{
					PktQuestUpdateNotify notify;
					notify.setAddOrUpdateQuests(*updateQuests);
					player->send(notify);
				});
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  트리거에 진입한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::_onTriggerEnter(CacheTx& cacheTx, const Trigger& trigger) const
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템 장착시 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::_onItemEquip(CacheTx& cacheTx, const Item& item)
{
	auto checker = [&item](const IfConditionDataBasePtr conditionData)
		{
			auto data = std::dynamic_pointer_cast<IfConditionDataItemEquip>(conditionData);
			if (!data)
				return false;

			if (data->getItemInfo() != &item.getInfo())
				return false;

			return true;
		};

	auto updateQuests = std::make_shared<std::vector<PktQuest>>();
	if (_onTaskUpdate(cacheTx, EIfCondition::ItemEquip, checker, nullptr, *updateQuests))
	{
		if (!updateQuests->empty())
		{
			auto player = _player.shared_from_this<Player>();
			cacheTx.ifSucceed(_player, [player, updateQuests]()
				{
					PktQuestUpdateNotify notify;
					notify.setAddOrUpdateQuests(*updateQuests);
					player->send(notify);
				});
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템 획득시 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::_onItemAcquire(CacheTx& cacheTx, const ItemInfo& info)
{
	auto checker = [&info, this](const IfConditionDataBasePtr conditionData)
		{
			auto data = std::dynamic_pointer_cast<IfConditionDataItemAcquire>(conditionData);
			if (!data)
				return false;

			if (data->getItemInfo() != &info)
				return false;

			std::vector<ItemPtr> findItems;
			int32_t invenCount = 0;

			if (_player.getInvenComponent().findItems(info, findItems))
			{
				for (auto item : findItems) {
					invenCount += item->getAccum();
				}
			}
			return data->getCount() < invenCount;
		};

	auto updateQuests = std::make_shared<std::vector<PktQuest>>();
	if (_onTaskUpdate(cacheTx, EIfCondition::ItemAcquire, checker, nullptr, *updateQuests))
	{
		if (!updateQuests->empty())
		{
			auto player = _player.shared_from_this<Player>();
			cacheTx.ifSucceed(_player, [player, updateQuests]()
				{
					PktQuestUpdateNotify notify;
					notify.setAddOrUpdateQuests(*updateQuests);
					player->send(notify);
				});
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트완료시 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::onQuestComplete(CacheTx& cacheTx, const QuestInfo& info, std::vector<PktQuest>& pktAddOrUpdate)
{
	_validateEnableQuest(cacheTx, pktAddOrUpdate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트가 완료가능한지 체크한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool QuestComponent::isCompleteEnable(const QuestInfo& questInfo) const
{
	auto iter = _quests.find(questInfo.getId());
	if (iter == _quests.end())
		return false;

	auto quest = iter->second;
	ENSURE(quest, return false);

	return (EQuestState)quest->getState() == EQuestState::Completable;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  시작가능한지 체크한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool QuestComponent::isStartEnable(const QuestInfo& questInfo) const
{
	auto iter = _quests.find(questInfo.getId());
	if (iter == _quests.end())
		return false;

	auto quest = iter->second;
	ENSURE(quest, return false);

	return (EQuestState)quest->getState() == EQuestState::Available;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  가능퀘스트를 갱신한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::_validateEnableQuest(const DataBaseTarget& db, std::vector<PktQuest>& pktAddOrUpdate)
{
	auto curTime = Core::Time::GetCurTime();

	for (const auto questInfo : QuestInfoManager::Instance().getInfos() | std::views::values)
	{
		/// 이전 퀘스트 완료 여부
		if (auto prevQuest = questInfo->getPrevQuest())
			if (_questCompletes.find(prevQuest) == _questCompletes.end())
				continue;

		/// 퀘스트완료 목록에 있는지여부
		if (_questCompletes.find(questInfo->getId()) != _questCompletes.end())
			continue;

		/// 진행가능/진행중인 퀘스트 여부
		auto iter = _quests.find(questInfo->getId());
		if (iter != _quests.end())
		{
			auto quest = iter->second;
			if ((EQuestState)quest->getState() == EQuestState::Progressing)
			{
				if (quest->getInfo().getClearConditionDatas().empty())
				{
					quest->setState((int8_t)EQuestState::Completable);
					if (!quest->update(db))
						WARN_LOG("auto complete update failed [pid:%llu, infoId:%u]", _player.getId(), quest->getInfoId());
				}
			}

			continue;
		}

		QuestPtr quest = std::make_shared<Quest>(*questInfo);
		quest->setOwnerId(_player.getId());
		quest->setInfoId(questInfo->getId());
		quest->setState((int8_t)EQuestState::Available);
		quest->setStartTime(curTime);

		if (quest->getInfo().getIsAutoStart())
		{
			quest->setState((int8_t)(quest->getInfo().getClearConditionDatas().empty()
				? EQuestState::Completable
				: EQuestState::Progressing));
		}

		if (!quest->insert(db))
		{
			WARN_LOG("auto start quest insert failed [pid:%llu, infoId:%u]", _player.getId(), quest->getInfoId());
			continue;
		}

		_quests.insert(std::make_pair(quest->getInfoId(), quest));

		PktQuest pktQuest;
		quest->exportTo(pktQuest);
		pktAddOrUpdate.emplace_back(std::move(pktQuest));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  가능퀘스트를 갱신한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::_validateEnableQuest(CacheTx& cacheTx, std::vector<PktQuest>& pktAddOrUpdate) const
{
	auto curTime = Core::Time::GetCurTime();
	auto player = _player.shared_from_this<Player>();

	for (const auto questInfo : QuestInfoManager::Instance().getInfos() | std::views::values)
	{
		/// 이전 퀘스트 완료 여부
		if (auto prevQuest = questInfo->getPrevQuest())
			if (_questCompletes.find(prevQuest) == _questCompletes.end())
				continue;

		/// 퀘스트완료 목록에 있는지여부
		if (_questCompletes.find(questInfo->getId()) != _questCompletes.end())
			continue;

		/// 진행가능/진행중인 퀘스트 여부
		if (_quests.find(questInfo->getId()) != _quests.end())
			continue;

		QuestPtr quest = std::make_shared<Quest>(*questInfo);
		quest->setOwnerId(_player.getId());
		quest->setInfoId(questInfo->getId());
		quest->setState((int8_t)EQuestState::Available);
		quest->setStartTime(curTime);

		if (quest->getInfo().getIsAutoStart())
		{
			quest->setState((int8_t)(quest->getInfo().getClearConditionDatas().empty()
				? EQuestState::Completable
				: EQuestState::Progressing));
		}

		auto cacheQuest = cacheTx.acquireObject(_player, quest);
		cacheQuest->insertCache();

		PktQuest pktQuest;
		cacheQuest->exportTo(pktQuest);
		pktAddOrUpdate.emplace_back(std::move(pktQuest));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  퀘스트와 관련한 대리자를 갱신한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void QuestComponent::_validateDelegate()
{
	if (!_player.checkThread())
		return;

	std::set<EIfCondition> nextIfs;
	for (const auto& [key, quest] : _quests)
		for (const auto& data : quest->getInfo().getClearConditionDatas())
			nextIfs.insert(data->getType());

	std::set<EIfCondition> removeIfs;
	std::set_difference(_checkIfs.begin(), _checkIfs.end(), nextIfs.begin(), nextIfs.end(),
		std::inserter(removeIfs, removeIfs.begin()));

	std::set<EIfCondition> addIfs;
	std::set_difference(nextIfs.begin(), nextIfs.end(), _checkIfs.begin(), _checkIfs.end(),
		std::inserter(addIfs, addIfs.begin()));

	if (removeIfs.empty() && addIfs.empty())
		return;

	_checkIfs = nextIfs;

	for (auto removeType : removeIfs)
	{
		switch (removeType)
		{
		case EIfCondition::ItemAcquire:
			if (_acquireItemHandle)
				_acquireItemHandle->invalidate();
			break;
		case EIfCondition::NpcInteractionByDelivery:
			if (_npcInteractionHandle)
				_npcInteractionHandle->invalidate();
			break;
		case EIfCondition::TriggerEnter:
			if (_triggerEnterHandle)
				_triggerEnterHandle->invalidate();
			break;
		default:
			break;
		}
	}

	for (auto addType : addIfs)
	{
		switch (addType)
		{
		case EIfCondition::ItemAcquire:
			_acquireItemHandle = _player.getItemAcquireDelegate().add([this](CacheTx& cacheTx, Player& self, const ItemInfo& info)
				{
					_onItemAcquire(cacheTx, info);
				});
			break;
		case EIfCondition::NpcInteractionByDelivery:
			_npcInteractionHandle = _player.getNpcInteractionDelegate().add([this](CacheTx& cacheTx, Player& self, const NpcInfo& info, EIfCondition conditionType)
				{
					_onInteractionNpc(cacheTx, info, conditionType);
				});
			break;
		case EIfCondition::TriggerEnter:
			_triggerEnterHandle = _player.getTriggerEnterDelegate().add([this](CacheTx& cacheTx, Player& self, const Trigger& trigger)
				{
					_onTriggerEnter(cacheTx, trigger);
				});

			break;
		default:
			break;
		}
	}
}
