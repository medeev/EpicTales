////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-3-31
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "FsmTypes.h"
#include "NpcStateBase.h"

class Player;

class NpcStateSpawn
	:
	public NpcStateBase
{
	typedef NpcStateBase super;

public:
	/// 생성자
	NpcStateSpawn(FsmComponent& fsm, Npc& npc, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override;

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* newState) override {}

	/// 스테이트가 갱신될 때를 처리한다
	virtual void onUpdate(int64_t curtimeValue) override;

	/// 전투스테이지 여부를 반환한다
	virtual bool isCombatState() const {
		return false;
	}
protected:
	int64_t _endTime; ///< 종료시간
};
