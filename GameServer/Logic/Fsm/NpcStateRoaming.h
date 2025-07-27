////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-3-30
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "FsmTypes.h"
#include "NpcStateBase.h"

class Npc;

class NpcStateRoaming
	:
	public NpcStateBase
{
	typedef NpcStateBase super;

	int64_t _nextMoveToTime; ///< 다음 로밍이동시간

public:
	/// 생성자
	NpcStateRoaming(FsmComponent& fsm, Npc& npc, EFsmStateType type);

	/// 스테이트에 진입할 때를 처리한다
	virtual void onEnter(IState* prevState, const StateInit* init) override;

	/// 스테이트가 갱신될 때를 처리한다
	virtual void onUpdate(int64_t curTimeValue) override;

	/// 전투스테이지 여부를 반환한다
	virtual bool isCombatState() const {
		return false;
	}

private:
	bool _startRoamingRandomLocation();
	
};
