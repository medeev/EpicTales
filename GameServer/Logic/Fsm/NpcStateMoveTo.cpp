

#include "Pch.h"
#include "NpcStateMoveTo.h"

#include <Core/Math/Numeric.h>
#include <Protocol/PktCommon.h>

#include "Actor/Npc.h"
#include "Config.h"
#include "FsmStateInit.h"
#include "Logic/Npc/NpcMoveComponent.h"
#include "Logic/NpcAi/NpcAiComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcStateMoveTo::NpcStateMoveTo(FsmComponent& fsm, Npc& npc, EFsmStateType type)
	:
	super(fsm, npc, type)
{
	_setUpdateTimeInterval(100);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	상태에 진입힌다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateMoveTo::onEnter(IState* prevState, const StateInit* init)
{
	super::onEnter(prevState, init);

	_setUpdateTimeInterval(100);
	_setLookTimeInterval(1000);

	ENSURE(init, return);
	auto stateInit = init->cast<NpcStateInitMoveTo>();
	ENSURE(stateInit, return);

	_isMoveToEnd = false;

	_moveToLocation = stateInit->_location;
	_permitRange = stateInit->_range;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  나갈때 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateMoveTo::onExit(IState* newState)
{
	if (!_isMoveToEnd)
		_setTaskLatentResult(EBTNodeResult::Failed);

	NpcStateBase::onExit(newState);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	상태를 업데이트한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateMoveTo::onUpdate(int64_t curtimeValue)
{
	if (!_isUpdateTime(curtimeValue))
		return;

	_setNextUpdateTime(curtimeValue);

	auto curLocation = _npc.getLocation();

	if (curLocation.getSquaredDistance2D(_moveToLocation) < _permitRange * _permitRange)
	{
		_isMoveToEnd = true;
		_setTaskLatentResult(EBTNodeResult::Succeeded);
		return;
	}

	int32_t moveSpeed = 240;

	auto& comp = _npc.getNpcMoveComponent();
	if (comp.getMovementType()
		== NpcMoveComponent::EMovementType::Position)
	{
		if (comp.getMoveToStatus() == EMoveToStatus::Moving)
		{
			if (_npc.getNpcMoveComponent().getPathPointEnd().equalsNealy2D(_moveToLocation))
				return;
		}
	}

	if (_npc.getNpcMoveComponent().moveTo(_moveToLocation) == EMoveToRet::FailFindPath)
	{
		WARN_LOG("npc find path failed");
	}

}
