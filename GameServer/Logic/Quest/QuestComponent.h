////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		컴포넌트 클래스
///
///	@ date		2024-5-9
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Core/Container/Delegate.h>
#include <Data/IfConditionData/IfConditionDataTypes.h>
#include <DB/Orms/OrmQuest.h>
#include <Protocol/Struct/PktQuest.h>
#include <Protocol/Struct/PktTypes.h>

#include "Actor/ActorTypes.h"
#include "Component/IComponent.h"
#include "DB/CacheTx.h"
#include "OrmSub/OrmSubType.h"

class Player;
class Item;
class ItemInfo;
class NpcInfo;
class Trigger;
class QuestInfo;
class QuestComponent
	:
	public IComponent
{
	friend Quest;
	typedef IComponent super;
	typedef std::function<bool(const IfConditionDataBasePtr)> ConditionCallback;
	typedef std::function<bool(CacheTx&, const IfConditionDataBasePtr, int32_t)> CompleteCallback;
protected:
	Player& _player; ///< 플레이어
	std::unordered_map<PktInfoId, QuestPtr> _quests;
	std::unordered_map<PktInfoId, QuestPtr> _questCompletes;
	std::set<EIfCondition> _checkIfs; ///< 현재 조건검사중인 목록
	Core::HandlePtr _acquireItemHandle;
	Core::HandlePtr _npcInteractionHandle;
	Core::HandlePtr _triggerEnterHandle;
	bool _init;

public:
	/// 생성자
	QuestComponent(Player& actor);

	/// 소멸자
	virtual ~QuestComponent();

	/// 진행중 퀘스트를 내보낸다.
	void exportTo(std::vector<PktQuest>& dst) const;

	/// 진행가능/진행중 퀘스트를 추가한다.
	void insertQuest(QuestPtr quest);

	/// 진행가능/진행중 퀘스트를 삭제한다.
	void deleteQuest(PktInfoId infoId);

	/// 완료 퀘스트에 추가한다.
	void insertCompletedQuest(QuestPtr quest);

	/// 퀘스트를 찾는다.
	const QuestPtr findQuest(PktInfoId infoId) const;
	QuestPtr findQuest(PktInfoId infoId);

	/// 완료 퀘스트를 찾는다.
	const QuestPtr findCompletedQuest(PktInfoId infoId) const;
	QuestPtr findCompletedQuest(PktInfoId infoId);

	/// 퀘스트를 DB에 추가한다.
	bool insertQuest(
		CacheTx& cacheTx,
		PktInfoId infoId,
		PktQuest& pktQuest) const;

	/// 퀘스트를 DB에 추가한다.
	bool startQuest(
		CacheTx& cacheTx,
		PktInfoId infoId,
		PktQuest& pktQuest) const;

	/// 퀘스트를 완료시킨다.
	bool completeQuest(
		CacheTx& cacheTx,
		const QuestInfo& questInfo,
		std::vector<PktQuest>&) const;

	/// 퀘스트완료시 처리한다.
	void onQuestComplete(
		CacheTx& cacheTx,
		const QuestInfo& info,
		std::vector<PktQuest>&);

	/// 퀘스트가 완료가능한지 체크한다.
	bool isCompleteEnable(const QuestInfo&) const;

	/// 퀘스트가 시작가능한지 체크한다.
	bool isStartEnable(const QuestInfo& questInfo) const;

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// DB객체로 초기화 한다
	virtual bool initializeDB(const DataBaseTarget& db) override;

	/// DB 객체로 초기화 된 이후에 처리한다.
	virtual void initializeDBPost(const DataBaseTarget& db) override;

	/// 정리 한다
	virtual void finalize() override;

	/// 룸진입후 처리한다.
	virtual void beginPlay() override;

protected:
	/// 테스크를 업데이트 할때 호출된다
	bool _onTaskUpdate(
		CacheTx& cacheTx,
		EIfCondition conditionType,
		ConditionCallback updateChecker,
		CompleteCallback completeChecker,
		std::vector<PktQuest>& updateQuests) const;

	/// 몬스터 킬시 처리한다.
	void _onKillMonster(
		CacheTx& cacheTx,
		PktInfoId npcInfoId) const;

	/// 아이템 장착시 처리한다.
	void _onItemEquip(CacheTx& cacheTx, const Item& item);

	/// 아이템 획득시 처리한다.
	void _onItemAcquire(CacheTx& cacheTx, const ItemInfo& info);

	/// Npc와 상호작용시 처리한다.
	void _onInteractionNpc(
		CacheTx& cacheTx,
		const NpcInfo& info,
		EIfCondition conditionType) const;

	/// 트리거에 자신이 진입할때 처리한다.
	void _onTriggerEnter(CacheTx& cacheTx, const Trigger& trigger) const;

	/// 설정된 가능퀘스트를 새로 갱신한다.
	void _validateEnableQuest(const DataBaseTarget& db, std::vector<PktQuest>&);
	void _validateEnableQuest(CacheTx& cacheTx, std::vector<PktQuest>&) const;

	/// 퀘스트와 관련한 대리자를 갱신한다.
	void _validateDelegate();
};
