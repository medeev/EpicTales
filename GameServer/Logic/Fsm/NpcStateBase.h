////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-4-8
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "../NpcAi/BTNodeTypes.h"
#include "FsmTypes.h"
#include "IState.h"

class AggroComponent;
class EffectComponent;
class Npc;
class NpcAiComponent;
class NpcMoveComponent;
class SkillComponent;
class NpcStateBase
	:
	public IState
{
	typedef IState super;

protected:
	Npc& _npc; ///< 엔피시
	int64_t _lookTimeInterval; ///< 업데이트 주기
	int64_t _nextlookTime; ///< 다음 업데이트 시간

public:
	/// 생성자
	NpcStateBase(FsmComponent& fsm, Npc& npc, EFsmStateType type);

	virtual void onEnter(IState* prevState, const StateInit* init) override {}

	/// 스테이트를 나갈때를 처리한다
	virtual void onExit(IState* newState) override {}

	//// 진입이후 처리한다.
	virtual void onEnterPost();

protected:
	void _setTaskLatentResult(EBTNodeResult result);

	/// 업데이트가 가능한지 여부를 반환한다
	bool _isLookTime(int64_t curTimeValue) const;

	/// 다음 업데이트 시간을 설정한다
	void _setNextLookTime(int64_t curtTimeValue);

	/// 타겟 찾기 간격을 설정한다.
	void _setLookTimeInterval(uint32_t t) {
		_lookTimeInterval = t;}

};
