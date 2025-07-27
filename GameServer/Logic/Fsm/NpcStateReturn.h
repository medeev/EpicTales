////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-3-30
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "FsmTypes.h"
#include "NpcStateBase.h"

class Npc;
class Player;

class NpcStateReturn
	:
	public NpcStateBase
{
	typedef NpcStateBase super;

	Vector _returnEndPos; // 돌아갈 최종위치
public:
	/// 생성자
	NpcStateReturn(FsmComponent& fsm, Npc& npc, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override;

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* newState) override {}

	/// 스테이트가 갱신될 때를 처리한다
	virtual void onUpdate(int64_t curtimeValue) override;
};
