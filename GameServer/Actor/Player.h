////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		플레이어
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Core/Container/Delegate.h>
#include <Core/Task/TaskCaller.h>
#include <Core/Task/TaskManager.h>
#include <Data/Info/InfoEnums.h>

#include "Actor.h"
#include "DB/BusyState.h"
#include "OrmSub/PlayerDB.h"
#include "User/UserTypes.h"
#include "Room/RoomThreadContext.h"

namespace Core
{
	class TaskManager;
};

class CacheTx;
class CharacterComponent;
class InventoryComponent;
class EquipComponent;
class QuestComponent;
class AchievementComponent;
class SkillComponent;
class MailComponent;
class CloseComponent;
class CharacterInfo;
class PktBase;
class PktPlayerData;
class PktPlayerSightInfo;
class Item;


class Player
	:
	public Actor
{
	typedef Actor super;

	/// 식별자 목록 타입 정의
	typedef std::unordered_set<PktObjId> ActorIdList;

	/// 정보식별자 목록 타입 정의
	typedef std::unordered_set<PktInfoId> InfoIdList;

public:
	using EquipDelegate = Core::Delegate<CacheTx&, Player&, const Item&>;
	using AcquireItemDelegate = Core::Delegate<CacheTx&, Player&, const ItemInfo&>;
	using NpcInteractionDelegate = Core::Delegate<CacheTx&, Player&, const NpcInfo&, EIfCondition>;
	using TriggerEnterDelegate = Core::Delegate<CacheTx&, Player&, const Trigger&>;
	using KillNpcDelegate = Core::Delegate<CacheTx&, Player&, const Npc&>;
	using LevelupDelegate = Core::Delegate<CacheTx&, Player&>;
public:
	enum class EDeliverEvent
	{
		Quest, ///< 퀘스트로 전달
	};
private:
	UserWeakPtr _userWeakPtr;        ///< 유저 위크 포인터
	ActorIdList _seeingActorIds;            ///< 현재보여지는 목록
	const CharacterInfo* _info; ///< 캐릭터 정보
	std::function<void(Player&)> _leaveCallback; ///< 방나갈때 호출되는 콜백
	EnvObjWeakPtr _ctrledEnvObj;
	std::unique_ptr<CharacterComponent>_characterComponent;
	std::unique_ptr<InventoryComponent> _inventoryComponent;
	std::unique_ptr<EquipComponent> _equipComponent;
	std::unique_ptr<QuestComponent> _questComponent;
	std::unique_ptr<AchievementComponent> _achievementComponent;
	std::unique_ptr<SkillComponent> _skillComponent;
	std::unique_ptr<MailComponent> _mailComponent;
	std::unique_ptr<CloseComponent> _closeComponent;
	int32_t _latency;

	EquipDelegate _equipOnDelegate; ///< 장착이벤트를 감지하는 대리자
	AcquireItemDelegate _acquireItemDelegate; ///< 아이템 획득 이벤트를 감지하는 대리자
	NpcInteractionDelegate _npcInteractionDelegate; ///< Npc상호작용 대리자
	TriggerEnterDelegate _triggerEnterDelegate; ///< 플레이어 트리거 진입대라지
	KillNpcDelegate _killNpcDelegate; ///< Npc 킬을이벤트를 감지하는 대리자

	std::shared_ptr<PlayerDB> _playerDb;
	uint32_t _lastPublicWorldInfoId; ///< 마지막 공용월드정보
	Vector _lastPublicWorldLocation; ///< 마지막 공용월드 위치
	const CharacterInfo* _characterInfo;
	
public:
	/// 생성자
	Player();

	/// 소멸자
	virtual ~Player();

	/// 유저를 설정한다
	void setUser(const UserPtr& user);

	/// 유저를 반환한다
	const UserPtr getUser() const;

	/// 유저를 반환한다
	UserPtr getUser();

	/// 마지막으로 진입한 공용월드정보를 설정한다.
	void setLastPublicWorldInfoId(uint32_t value) {
		_lastPublicWorldInfoId = value;
	}
	void setLastPublicWorldLocation(const Vector& value) {
		_lastPublicWorldLocation = value;
	}
	Vector getDBLocation() const;

	/// 닉네임을 반환한다.
	const std::string& getNick() const;

	/// 케릭터에 대한 get/set입니다.
	CharacterComponent& getCharacterComponent() {
		return *_characterComponent;
	}
	const CharacterComponent& getCharacterComponent() const {
		return *_characterComponent;
	}

	/// 스킬에 대한 get/set입니다.
	SkillComponent& getSkillComponent() {
		return *_skillComponent;
	}
	const SkillComponent& getSkillComponent() const {
		return *_skillComponent;
	}

	/// 메일에 대한 get/set입니다.
	MailComponent& getMailComponent() {
		return *_mailComponent;
	}
	const MailComponent& getMailComponent() const {
		return *_mailComponent;
	}

	/// 인벤토리에 대한 get/set입니다.
	InventoryComponent& getInvenComponent() {
		return *_inventoryComponent;
	}
	const InventoryComponent& getInvenComponent() const {
		return *_inventoryComponent;
	}

	/// 장비장착에 대한 get/set입니다.
	EquipComponent& getEquipComponent() {
		return *_equipComponent;
	}
	const EquipComponent& getEquipComponent() const {
		return *_equipComponent;
	}

	/// 퀘스트에 대한 get/set입니다.
	QuestComponent& getQuestComponent() {
		return *_questComponent;
	}
	const QuestComponent& getQuestComponent() const {
		return *_questComponent;
	}

	/// 업적을 반환합나다.
	AchievementComponent& getAchievementComponent() {
		return *_achievementComponent;
	}
	const AchievementComponent& getAchievementComponent() const {
		return *_achievementComponent;
	}

	/// 종료처리에 대한 컴포넌트의 get/set입니다
	CloseComponent& getCloseComponent() {
		return *_closeComponent;
	}
	const CloseComponent& getCloseComponent() const {
		return *_closeComponent;
	}

	/// 몬스터 킬이벤트 대리자를 반환한다.
	KillNpcDelegate& getKillOnDelegate() {
		return _killNpcDelegate;
	}

	/// 아이템 장착 대리자를 반환한다.
	EquipDelegate& getItemEquipDelegate() {
		return _equipOnDelegate;
	}

	/// 아이템 획득 대리자를 반환한다.
	AcquireItemDelegate& getItemAcquireDelegate() {
		return _acquireItemDelegate;
	}

	/// Npc상호작용 대리자를 반환한다
	NpcInteractionDelegate& getNpcInteractionDelegate() {
		return _npcInteractionDelegate;
	}

	/// 플레이어 트리기 전입 대리자를 반환한다
	TriggerEnterDelegate getTriggerEnterDelegate() const {
		return _triggerEnterDelegate;
	}

	/// 레이턴시에 대한 get/set입니다.
	void setLatency(int32_t latency);
	int32_t getLatency() const {
		return _latency;
	}

	const CharacterInfo* getCharacterInfo() const {
		return _info;
	}
	void setCharacterInfo(const CharacterInfo* info) {
		_info = info;
	}


	/// 현재월드를 가져온다.
	int32_t getCurWorldInfoId() const;

	/// 현재월드를 설정한다.
	void setCurWorldInfoId(int32_t);


	/// 패킷을 보낸다
	bool send( const PktBase& pktBase) const;

	/// 월드에 입장한다
	void enterWorld(WorldPtr world, const std::function<void(WorldPtr, PlayerPtr)>& callback);

	/// 월드에서 나간다
	void leaveWorld(const std::function<void(Player&)>& callback);

	/// 재접속 오래된 플레이어 처리한다.
	void reconnectOldPlayerProcess(RoomThreadContextRef);

	/// 끊길때 처리한다.
	void disconnnectSave(RoomThreadContextRef);


	/// 룸에서 작업을 한다.
	void runTask(const Core::TaskCaller& taskCaller, const Core::TaskManager::Task& task);

	/// 나기기 콜백을 설정한다.
	void setRoomLeaveCallback(const std::function<void(Player&)>& callback) {
		_leaveCallback = callback;
	}

	/// 나가기 콜백을 실행한다.
	void runRoomLeaveCallback() {
		if (_leaveCallback) {
			_leaveCallback(*this);
			_leaveCallback = nullptr;
		}
	}

	/// 플레이어서 조작중인 대상을 설정한다.
	void setCtrledEnvObj(EnvObjPtr envObj) {
		_ctrledEnvObj = envObj;
	}
	EnvObjPtr getCtrledEnvObj() {
		return _ctrledEnvObj.lock();
	}

	const BusyStateUpdaterPtr createBusyUpdater() const;

public:
	/// 초기화 한다
	virtual void initialize() override;

	/// 정리 한다
	virtual void finalize() override;

	/// 초기화한다.
	bool initializeDB(const DataBaseTarget& db);

	/// 갱신한다
	virtual void update(int64_t curTime) override;

	/// 반지름을 가져온다.
	virtual float getRadius() const override;

	/// 높이 중점을 가져온다.
	virtual float getHalfHeight() const override;

	/// 보여지는 액터인지여부
	bool isSeeingActorId(PktObjId id) const;

	/// 보여지는 목록에 추가한다.
	void addSeeingActorId(PktObjId id);

	/// 보여지는 목록에서 제거한다.
	void removeSeeingActorId(PktObjId id);

	/// 보여지는 목록을 모두 제거한다.
	void removeSeeingAll();

	/// 보여지는 숫자를 반환한다
	size_t getSeeingCount() const;

	/// 루트포지션값을 반환한다.
	Vector getRootLocation() const override;

public:
	/// 작업 관리자를 반환한다
	Core::TaskManagerPtr getTaskManager();

public:
	void exportTo(PktPlayerSightInfo& dest) const;
	void exportTo(PktPlayerData& dest) const;

	/// 정보를 내보낸다
	virtual void exportTo(PktSightEnterNotify& dest) const;

public:
	virtual void endPlay() override;
	virtual void endPlayPost() override;
	virtual void beginPlay() override;

	void broadcastKillNpc(CacheTx& cacheTx, Npc& npc);
	void broadcastEquipItem(CacheTx& cacheTx, Item& item);
	void broadcastAcquireItem(CacheTx& cacheTx, const ItemInfo& info);
	void broadcastNpcInteraction(CacheTx& cacheTx, const NpcInfo& info, EIfCondition conditionType);
	virtual void broadcastTriggerEnter(const Trigger& trigger) override;
};

