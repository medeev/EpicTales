////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-3-22
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "FsmTypes.h"
#include "PlayerStateBase.h"

class Player;

class PlayerStateIdle
	:
	public PlayerStateBase
{
	typedef PlayerStateBase super;

public:
	/// 생성자
	PlayerStateIdle(FsmComponent& fsm, Player& owner, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override
	{
		super::onEnter(prevState, init);
	}

	/// 스테이트를 나갈 때를 처리한다
	virtual void onExit(IState* newState) override
	{
		super::onExit(newState);
	}

	/// 스테이트가 갱신될 때를 처리한다
	virtual void onUpdate(int64_t curtimeValue) override {}
};
