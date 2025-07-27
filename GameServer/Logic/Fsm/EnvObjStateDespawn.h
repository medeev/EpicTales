////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-4-11
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "EnvObjStateBase.h"
#include "FsmTypes.h"

class EnvObjStateDespawn
	:
	public EnvObjStateBase
{
	typedef EnvObjStateBase super;

	int64_t _despawnEndTime;
public:
	/// 생성자
	EnvObjStateDespawn(FsmComponent& fsm, EnvObj& gadget, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override;

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* newState) override {}

	/// 스테이트가 갱신될 때를 처리한다
	virtual void onUpdate(int64_t curtimeValue) override;
};
