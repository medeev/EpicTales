////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-3-31
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "EnvObjStateBase.h"
#include "FsmTypes.h"

class EnvObj;

class EnvObjStateSpawn
	:
	public EnvObjStateBase
{
	typedef EnvObjStateBase super;

public:
	/// 생성자
	EnvObjStateSpawn(FsmComponent& fsm, EnvObj& npc, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override;

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* newState) override {}

	/// 스테이트가 갱신될 때를 처리한다
	virtual void onUpdate(int64_t curtimeValue) override;

protected:
};
