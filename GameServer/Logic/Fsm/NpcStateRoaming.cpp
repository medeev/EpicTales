#include "Pch.h"
#include "NpcStateRoaming.h"

#include <Data/Info/NpcInfo.h>
#include <Data/Info/WorldInfo.h>

#include "Actor/Npc.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcStateRoaming::NpcStateRoaming(FsmComponent& fsm, Npc& npc, EFsmStateType type)
	:
	super(fsm, npc, type)
{
	_setUpdateTimeInterval(200);
	_setLookTimeInterval(200);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트에 진입할 때를 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateRoaming::onEnter(IState* prevState, const StateInit* init)
{
	super::onEnter(prevState, init);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트를 업데이트 한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateRoaming::onUpdate(int64_t curTimeValue)
{
	if (!_isUpdateTime(curTimeValue))
		return;

	_setNextUpdateTime(curTimeValue);

	if (_isLookTime(curTimeValue))
	{
		auto cognitionDist = (float)_npc.getInfo().getCognitionDist();

		_setNextLookTime(curTimeValue);
	}

	
	{
		auto& comp = _npc.getNpcMoveComponent();
		auto moveStatus = comp.getMoveToStatus();
		if (moveStatus == EMoveToStatus::MoveCompleted ||
			moveStatus == EMoveToStatus::Stop)
		{
			if (!_nextMoveToTime)
				_nextMoveToTime = curTimeValue + Core::Numeric::Random(2500, 3500);
		}
	}

	if (_nextMoveToTime && _nextMoveToTime < curTimeValue)
	{
		if (_startRoamingRandomLocation())
			_nextMoveToTime = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  랜덤한 위치로 로밍을 시작한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcStateRoaming::_startRoamingRandomLocation()
{
	if (auto world = _npc.getWorld())
	{
		Vector location;
		world->getInfo().getRandomLocationPathConnectedInDonut(_npc.getLocation(), 100.f, 220.f, location);
		location.z += _npc.getHalfHeight();
		if (_npc.getNpcMoveComponent().moveTo(location) == EMoveToRet::Success)
			return true;
	}

	return false;
}
