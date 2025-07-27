////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-8-28
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "FsmTypes.h"
#include "NpcStateBase.h"

class Npc;
class Player;

class NpcStateMoveTo
	:
	public NpcStateBase
{
	typedef NpcStateBase super;

public:
	/// 생성자
	NpcStateMoveTo(FsmComponent& fsm, Npc& npc, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override;

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* newState) override;

	/// 스테이트가 갱신될 때를 처리한다
	virtual void onUpdate(int64_t curtimeValue) override;

private:
	bool _isMoveToEnd;
	Vector _moveToLocation;
	int32_t _permitRange;
};
