////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-3-28
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "FsmTypes.h"
#include "PlayerStateBase.h"

class EnvObj;
class Player;

class PlayerStateCtrledEnvObj
	:
	public PlayerStateBase
{
	typedef PlayerStateBase super;

private:
	bool _isCtrlEnd; ///< 컨트롤 종료 여부
	int64_t _ctrlEndTime; ///< 컨트롤 종료 시간

public:
	/// 생성자
	PlayerStateCtrledEnvObj(FsmComponent& fsm, Player& owner, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override;

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* newState) override;

	/// 스테이트가 갱신될 때를 처리한다
	virtual void onUpdate(int64_t curtimeValue) override;

private:
	void _controlComplete();
	void _controlCompleteFailed();

};
