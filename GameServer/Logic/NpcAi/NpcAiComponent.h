////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		컴포넌트 클래스
///
///	@ date		2024-3-14
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Data/Info/InfoEnums.h>

#include "BTNode.h"
#include "BTNodeTypes.h"
#include "BTRunningTaskData.h"
#include "Component/IComponent.h"
#include "Logic/Cooltime/CoolTimeManager.h"

class Trigger;
class SkillInfo;
class NpcAiComponent : public IComponent, public CoolTimeManager
{
public:
	typedef std::unordered_map< std::string, int32_t > StringValues; /// 변수명/값 정의
	typedef std::unordered_map< std::string, std::vector< Vector > > StringPositions; /// 변수명/값 정의
	
private:
	Npc& _npc; ///< Npc
	int64_t _nextUpdate; ///< 다음업데이트갱신주기
	BTNode* _btTree; ///< 행동트리
	StringValues _stringValues; ///< 변수 체크 목록
	StringPositions _stringPositions; ///< 위치변수 체크 목록
	int64_t _combatStartTime; ///< 전투 시작 시간
	BTRunningTaskData _runningNodeData;
	bool _isUpdateDirty;
	std::map<const TaskNode*, int32_t> _taskSuccCounts;
	std::map<const TaskNode*, int64_t> _taskCooltime;
	ActorWeakPtr _owner; ///< 소환몬스터일 경우 소환 caster
	int32_t _chaseTargetInterval; ///< 추적대상확인 주기
	int64_t _lastChaseTargetRefreshTime; ///< 마지막 추적대상 확인 시간
	ActorWeakPtr _target; ///< behaviorTree타겟
	int64_t _curTimeValue;
	const BTNode* _forceSuccessNode;
	std::map<const BTNode*, int64_t> _decoTimeLimits; ///< 데코레이트 Wating타임맵
	
public:
	/// 생성자
	NpcAiComponent(Npc& npc);

	/// 소멸자
	virtual ~NpcAiComponent();

	/// AI를 초기화한다.
	void reset();

	/// 추적대상 확인 주기를 설정한다.
	void setChaseTargetInterval(int32_t interval) {
		_chaseTargetInterval = interval;
	}
	int32_t getChaseTargetInterval() const {
		return _chaseTargetInterval;
	}
	
	/// 전투 시작 시간을 반환한다.
	int64_t getCombatStartTime() const {
		return _combatStartTime;
	}

	/// 진행중 Task의 결과값을 설정한다.
	void setTaskLatentResult(EBTNodeResult ret);

	/// Npc BehaviorTree Task가 결과를 생성할때 처리한다.
	void onTaskResult(const TaskNode& task, EBTNodeResult& ret);

	/// Npc Task 의 공통체크로직에 제한여부를 반환한다.
	bool checkEnableTask(const TaskNode& task, EBTNodeResult& ret) const;

	/// Npc Decorate 의 선처리관련값을 설정한다.
	void setDecorateForceSuccessTask(const BTNode& node);

	/// Npc Decorate 중 TimeLimit관련 값을 반환한다.
	bool getDecorateInTime(const BTNode& node, int64& outMSec) const;

	/// Npc Decorate 중 TimeLimit관련 값을 설정한다.
	void setDecorateInTime(const BTNode& node, const int32_t mSec);

	void setOwner(ActorPtr owner) {
		_owner = owner;
	}
	ActorPtr getOwner() {
		return _owner.lock();
	}

	void updateOnceAi() {
		_isUpdateDirty = true;
	}

	bool isAttackerInRange(Actor* target, float minAngle, float maxAngle) const;
	bool isInAngleRange(Actor* target, EAngleRangeType& outAngleType) const;
	bool isTriggerEnterEnemy(int32_t checkCount, Trigger& trigger);
	
	/// 키에 대한 값을 설정/가져오는 함수입니다.
	bool getVariable(const std::string& key, int32_t& value) const;
	void setVariable(const std::string& key, int32_t value);
	void addVaraible(const std::string& key, int32_t value);
	bool getVariableLocation(const std::string& key, std::vector<Vector>& value) const;
	void setVariableLocation(const std::string& key, const Vector& value);
	void addVaraibleLocation(const std::string& key, const Vector& value);
	void clearVariableLocation(const std::string& key);
	void addVariableDynimicLocation(const std::string& key, bool isAllPlayer, bool isAllPlayerFront);

	void printTask(const TaskNode& node, EBTNodeResult result) const;

	void enterCombat();
	void exitCombat();

protected:
	/// Npc Task 제한횟수에 걸린지 여부를 반환한다.
	bool _isLimitTaskCount(const TaskNode& task, const int32_t limitCount) const;

	bool _isChaseTargetRefresh(int64_t curTimeValue) const;
	void _setLastChaseTargetRefreshTime(int64_t curTimeValue);

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// 정리 한다
	virtual void finalize() override;

	/// 갱신 한다
	virtual void update(int64_t curTime) override;
};
