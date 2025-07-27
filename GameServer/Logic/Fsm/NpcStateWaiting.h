////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-8-7
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "FsmTypes.h"
#include "NpcStateBase.h"

class NpcStateWaiting
	:
	public NpcStateBase
{
	typedef NpcStateBase super;
public:

public:
	NpcStateWaiting(FsmComponent& fsm, Npc& owner, EFsmStateType type);

public:
	virtual void onEnter(IState* prevState, const StateInit* init) override;
	virtual void onExit(IState* newState) override;
	virtual void onUpdate(int64_t curtimeValue) override;

	/// 전투스테이지 여부를 반환한다
protected:
	bool _isWaitingEnd;
	int64_t _waitingEndTime;
};