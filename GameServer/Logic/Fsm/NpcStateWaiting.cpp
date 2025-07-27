

#include "Pch.h"
#include "NpcStateWaiting.h"

#include "Actor/Npc.h"
#include "FsmStateInit.h"
#include "Logic/Fsm/FsmComponent.h"
#include "Logic/Npc/NpcMoveComponent.h"
#include "Logic/NpcAi/NpcAiComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcStateWaiting::NpcStateWaiting(FsmComponent& fsm, Npc& owner, EFsmStateType type)
	:
	super(fsm, owner, type)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스킬상태에 진입한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateWaiting::onEnter(IState* prevState, const StateInit* init)
{
	super::onEnter(prevState, init);

	auto prevNpcState = dynamic_cast<NpcStateBase*>(prevState);

	if (!init)
		return;

	auto initWait = init->cast<NpcStateInitWait>();
	if (!initWait)
		return;

	_isWaitingEnd = false;
	_waitingEndTime = initWait->endTime;

	if (prevState && prevState->getStateType() == EFsmStateType::NpcStateWaiting)
		WARN_LOG("current npc state NpcStateWaiting but enter new NpcStateWaiting");

	_npc.getNpcMoveComponent().stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스킬상태를 나갈때 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateWaiting::onExit(IState* newState)
{
	if (!_isWaitingEnd)
		_setTaskLatentResult(EBTNodeResult::Failed);

	super::onExit(newState);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스킬상태 갱신
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateWaiting::onUpdate(int64_t curTimeValue)
{
	if (_waitingEndTime && _waitingEndTime < curTimeValue)
	{
		_isWaitingEnd = true;
		_setTaskLatentResult(EBTNodeResult::Succeeded);
	}
}
