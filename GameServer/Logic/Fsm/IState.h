////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 클래스
///
///	@ date		2024-3-21
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Actor/ActorTypes.h"
#include "FsmTypes.h"

class Actor;
class StateInit;
class FsmComponent;

class IState
{
protected:
	FsmComponent& _fsm;
	Actor& _owner;
	EFsmStateType _type;
	int64_t _updateTimeInterval;
	int64_t _nextUpdateTime;

public:
	IState(FsmComponent& fsm, Actor& owner, EFsmStateType type);
	virtual ~IState() {}

	virtual void onEnter(IState *prevState, const StateInit* init) = 0;
	virtual void onEnterPost() {}
	virtual void onExit(IState *newState) = 0;
	virtual void onUpdate(int64_t curtimeValue) = 0;

	EFsmStateType getStateType() const {
		return _type;
	}

protected:
	void _setUpdateTimeInterval(int32_t mSec);
	bool _isUpdateTime(int64_t curTimeValue) const;
	void _setNextUpdateTime(int64_t curtTimeValue);
	

};
