////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-4-8
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "PlayerStateBase.h"

#include "FsmTypes.h"

class EffectComponent;
class Player;
class PlayerStateBase
	:
	public IState
{
	typedef IState super;

protected:
	Player& _player;
public:
	/// 생성자
	PlayerStateBase(FsmComponent& fsm, Player& actor, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override;

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* prevState) override {}

protected:
};
