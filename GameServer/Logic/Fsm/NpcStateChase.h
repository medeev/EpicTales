////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-3-30
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "FsmTypes.h"
#include "NpcStateBase.h"

const float kChaseUpdateCheckMaxMSec = 1.0;  // 업데이트 주기 최대
const float kChaseUpdateCheckMinMSec = 0.2f; // 업데이트 주기 최소

class Npc;
class Player;

class NpcStateChase
	:
	public NpcStateBase
{
	typedef NpcStateBase super;

public:
	/// 생성자
	NpcStateChase(FsmComponent& fsm, Npc& npc, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override;

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* newState) override;

	/// 스테이트가 갱신될 때를 처리한다
	virtual void onUpdate(int64_t curtimeValue) override;

private:

	bool _isReturn(ActorPtr chaseTarget, int64_t curTimeValue) const;
};
