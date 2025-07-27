#include "Pch.h"
#include "NpcAiComponent.h"

#include <Core/Util/StringUtil.h>
#include <Data/Info/NpcInfo.h>
#include <Protocol/PktCommon.h>

#include "Actor/Npc.h"
#include "BTNodeManager.h"
#include "Config.h"
#include "Logic/Fsm/FsmComponent.h"
#include "Logic/Npc/NpcMoveComponent.h"
#include "Logic/WorldEntity/Trigger.h"
#include "Room/Room.h"

static constexpr int64_t g_aiUpdateTick = 250;
static constexpr int64_t g_aiBossUpdateTick = 100;
static constexpr int32_t g_aiDefaultChaseTargetInterval = 2500;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcAiComponent::NpcAiComponent(Npc& npc)
	: IComponent(EComponentType::NpcAi, npc, EComponentUpdate::kYes),
	_npc(npc),
	_nextUpdate(0),
	_combatStartTime(0),
	_isUpdateDirty(false),
	_chaseTargetInterval(g_aiDefaultChaseTargetInterval),
	_lastChaseTargetRefreshTime(0)
{
	_btTree = BTNodeManager::Instance().getBehaviorTree(npc.getInfo().getId());

	reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcAiComponent::~NpcAiComponent()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::initialize()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		정리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::finalize()
{
	reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		갱신한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::update(int64_t curTime)
{
	_curTimeValue = curTime;

	if (!_isUpdateDirty)
	{
		if (curTime < _nextUpdate)
			return;

		_nextUpdate = curTime + g_aiUpdateTick;

		if (_npc.getState() != EFsmStateType::NpcStateChase)
			return;
	}

	_isUpdateDirty = false;

	if (_btTree)
	{
		const auto btRet = _btTree->execute(_npc, _runningNodeData);
		if (btRet != EBTNodeResult::InProgress)
			_runningNodeData.reset();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	AI를 초기화한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::reset()
{
	_chaseTargetInterval = g_aiDefaultChaseTargetInterval;
	_stringValues.clear();
	_stringPositions.clear();
	_combatStartTime = 0;
	_isUpdateDirty = false;
	_taskSuccCounts.clear();
	_taskCooltime.clear();
	_forceSuccessNode = nullptr;
	_decoTimeLimits.clear();
}

bool NpcAiComponent::_isChaseTargetRefresh(int64_t curTimeValue) const
{
	if (_lastChaseTargetRefreshTime == 0 || _lastChaseTargetRefreshTime + _chaseTargetInterval < curTimeValue)
		return true;

	return false;
}

void NpcAiComponent::_setLastChaseTargetRefreshTime(int64_t curTimeValue)
{
	_lastChaseTargetRefreshTime = curTimeValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  진행중 Task의 결과값을 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::setTaskLatentResult(EBTNodeResult ret)
{
	_runningNodeData.setLatentResult(ret);
	_isUpdateDirty = true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc BehaviorTree Task가 결과를 생성할때 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::onTaskResult(const TaskNode& task, EBTNodeResult& ret)
{
	auto room = _npc.getRoom();
	if (!room)
	{
		ret = EBTNodeResult::Failed;
		return;
	}

	if (ret == EBTNodeResult::Succeeded)
	{
		const auto& checkCommon = task.getCheckCommonData();

		if (checkCommon.excuteLimit)
		{
			auto& count = _taskSuccCounts[&task];
			count++;
		}

		if (checkCommon.coolTimeMSec > 0)
			_taskCooltime[&task] = room->getTime() + checkCommon.coolTimeMSec;

		task.print(_npc, ret);
	}
	else if (ret == EBTNodeResult::Failed)
	{
		if (_forceSuccessNode == &task)
		{
			ret = EBTNodeResult::Succeeded;
			task.print(_npc, ret);
		}
	}
	else if (ret == EBTNodeResult::InProgress)
	{
		task.print(_npc, ret);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc Task 의 공통체크로직에 제한여부를 반환한다.(false 리턴일때 ret값 참조)
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcAiComponent::checkEnableTask(const TaskNode& task, EBTNodeResult& ret) const
{
	auto room = _npc.getRoom();
	ENSURE(room, false);

	const auto& checkCommon = task.getCheckCommonData();

	if (auto excuteLimit = checkCommon.excuteLimit)
	{
		if (_isLimitTaskCount(task, excuteLimit))
		{
			ret = checkCommon.limitReturn ?
				EBTNodeResult::Succeeded : EBTNodeResult::Failed;

			return false;
		}
	}

	if (checkCommon.coolTimeMSec > 0)
	{
		auto iter = _taskCooltime.find(&task);
		if (iter != _taskCooltime.end())
		{
			auto endtime = iter->second;
			if (room->getTime() < endtime) {
				ret = EBTNodeResult::Failed;
				return false;
			}
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  테스크가 실패될때 성공으로 반환되게 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::setDecorateForceSuccessTask(const BTNode& node)
{
	_forceSuccessNode = &node;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief   Npc Decorate 중 TimeLimit관련 값을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcAiComponent::getDecorateInTime(const BTNode& node, int64& outMSec) const
{
	auto iter = _decoTimeLimits.find(&node);
	if (iter == _decoTimeLimits.end())
		return false;

	outMSec = iter->second;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc Decorate 중 TimeLimit관련 값을 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::setDecorateInTime(const BTNode& node, const int32_t mSec)
{
	auto room = _npc.getRoom();
	if (!room)
		return;

	_decoTimeLimits[&node] = room->getTime() + mSec;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc Task 성공횟수가 제한에 걸렸는지 여부
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcAiComponent::_isLimitTaskCount(const TaskNode& task, const int32_t limitCount) const
{
	auto iter = _taskSuccCounts.find(&task);
	if (iter == _taskSuccCounts.end())
		return false;

	return iter->second >= limitCount;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  키값에 대한 값을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcAiComponent::getVariable(const std::string& key, int32_t& value) const
{
	auto iter = _stringValues.find(key);
	if (iter == _stringValues.end())
		return false;

	value = (*iter).second;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  키에 대한 값을 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::setVariable(const std::string& key, int32_t value)
{
	_stringValues[key] = value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  키에 대한 값을 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::addVaraible(const std::string& key, int32_t value)
{
	auto& curValue = _stringValues[key];
	curValue += value;
}

bool NpcAiComponent::getVariableLocation(const std::string& key, std::vector<Vector>& value) const
{
	auto iter = _stringPositions.find(key);
	if (iter == _stringPositions.end())
		return false;

	value = (*iter).second;

	return true;
}

void NpcAiComponent::setVariableLocation(const std::string& key, const Vector& value)
{
	_stringPositions[key].clear();
	_stringPositions[key].push_back(value);
}

void NpcAiComponent::addVaraibleLocation(const std::string& key, const Vector& value)
{
	_stringPositions[key].push_back(value);
}

void NpcAiComponent::clearVariableLocation(const std::string& key)
{
	_stringPositions[key].clear();
}

void NpcAiComponent::addVariableDynimicLocation(const std::string& key, bool isAllPlayer, bool isAllPlayerFront)
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  디버깅 정보를 출력한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::printTask(const TaskNode& node, EBTNodeResult result) const
{
	//DEBUG_LOG("Name:%s %s(%s)",
	//	_npc.getInfo().getNick().c_str(),
	//	node.getNodeName().c_str(), convert(result).c_str());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  전투에 진입한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::enterCombat()
{
	_combatStartTime = Core::Time::GetCurTimeMSec();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  전투에서 나간다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcAiComponent::exitCombat()
{
	if (_owner.lock())
	{
		_npc.setRoomRemovable(true);
	}
	
	reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  대상이 Npc를 중심으로 주어진 각도 사이에 있는지 여부
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcAiComponent::isAttackerInRange(Actor* target, float minAngle, float maxAngle) const
{
	ENSURE(target, return false);

	Vector foward = _npc.getDirection();
	auto dirToTarget = target->getLocation() - _npc.getLocation();
	dirToTarget.z = 0;
	dirToTarget.normalize();

	float dot;
	/// 거의 같으면 내적이 1보다 커지져 Degree 가 nan이 발생한다. 거의 같을땐 내적 1이라고 하자.
	if (dirToTarget.equalsNealy(foward))
		dot = 1.f;
	else
		dot = Vector::Dot(foward, dirToTarget);

	float angleBetween = Core::Numeric::ToDegree(Core::Numeric::Acos(dot));
	
	// NPC와 대상 사이의 각도가 NPC의 오른쪽 (0~180도) 쪽에 있는지 체크한다.
	if (Vector::Cross(foward, dirToTarget).z < 0)
		angleBetween = 360.0f - angleBetween;

	INFO_LOG("angleBetween: %.f", angleBetween);

	minAngle = Core::Numeric::Fmod(minAngle + 360.0f, 360.0f);
	maxAngle = Core::Numeric::Fmod(maxAngle + 360.0f, 360.0f);

	if (minAngle <= maxAngle)
	{
		// 단순히 범위 내에 있는지 확인한다.
		return angleBetween >= minAngle && angleBetween <= maxAngle;
	}
	else
	{
		// 각도 범위가 360도를 넘어가는 경우한다.
		return angleBetween >= minAngle || angleBetween <= maxAngle;
	}
}

bool NpcAiComponent::isInAngleRange(Actor* target, EAngleRangeType& outAngleType) const
{
	ENSURE(target, return false);

	for (const auto& angle : _npc.getInfo().getAngleRangeTypes())
	{
		auto halfCentral = angle.centralAngle / 2;
		auto startAngle = angle.startAngle - halfCentral;
		auto endAngle = angle.startAngle + halfCentral;

		if (isAttackerInRange(target, (float)startAngle, (float)endAngle))
		{
			outAngleType = angle.type;

			return true;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  트리거에 적군이 들어가 있는지 체크한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcAiComponent::isTriggerEnterEnemy(int32_t checkCount, Trigger& trigger)
{
	Actors outActor;
	trigger.findOverlapActors(outActor);

	int32_t count = 0;
	for (const auto& actor : outActor)
	{
		if (actor->getRelation(_npc) == ETeamRelation::Enemy)
			++count;
	}

	return count <= checkCount;
}
