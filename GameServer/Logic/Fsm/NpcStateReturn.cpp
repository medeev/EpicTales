
#include "Pch.h"
#include "NpcStateReturn.h"

#include "Actor/Npc.h"
#include "Actor/Player.h"
#include "Room/Room.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcStateReturn::NpcStateReturn(FsmComponent& fsm, Npc& npc, EFsmStateType type)
	:
	super(fsm, npc, type)
{
	_updateTimeInterval = 1000;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트에 진입할 때를 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateReturn::onEnter(IState* prevState, const StateInit* init)
{
	super::onEnter(prevState, init);

	_npc.exitCombat();

	Vector returnPos;
	if (_npc.getLocationByType(NpcPositionType::CombatStartPos, returnPos))
	{
		if (_npc.getNpcMoveComponent().moveTo(returnPos) == EMoveToRet::Success)
		{
			_returnEndPos = _npc.getNpcMoveComponent().getPathPointEnd();
		}
		else
		{
			_npc.getNpcMoveComponent().stop();
			_returnEndPos = _npc.getRootLocation();
		}
	}
	else
	{
		_npc.getNpcMoveComponent().stop();
		_returnEndPos = _npc.getRootLocation();
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트가 갱신될 때를 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcStateReturn::onUpdate(int64_t curtimeValue)
{
	if (!_isUpdateTime(curtimeValue))
		return;

	_setNextUpdateTime(curtimeValue);

	auto elapsedTime = _fsm.getElapsedTime(curtimeValue);
	if (elapsedTime > 10000)
	{
		const auto& pos = _npc.getLocation();
		const auto& transform = _npc.getSpawnTransform();

		_npc.getNpcMoveComponent().teleport(transform);

		_fsm.changeState(EFsmStateType::NpcStateIdle);
		return;
	}

	if (_npc.getRootLocation().getSquaredDistance2D(_returnEndPos) < 1.f)
		_fsm.changeState(EFsmStateType::NpcStateIdle);
}

