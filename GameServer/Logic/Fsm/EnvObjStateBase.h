////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-3-31
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "FsmTypes.h"
#include "IState.h"

class EnvObj;

class EnvObjStateBase
	:
	public IState
{
	typedef IState super;

protected:
	EnvObj& _envObj;	///< 가젯

public:
	/// 생성자
	EnvObjStateBase( FsmComponent& fsm, EnvObj& gadget, EFsmStateType type );

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override {}

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* newState) override {}
};
