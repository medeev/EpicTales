

#include "Pch.h"
#include "FsmComponent.h"

#include <Data/Info/InfoEnumsConv.h>

#include "Actor/Actor.h"
#include "Actor/Player.h"
#include "Component/Entity.h"
#include "NpcStateChase.h"
#include "NpcStateIdle.h"
#include "NpcStateMoveTo.h"
#include "NpcStateReturn.h"
#include "NpcStateRoaming.h"
#include "NpcStateSpawn.h"
#include "NpcStateWaiting.h"
#include "PlayerStateCtrledEnvObj.h"
#include "PlayerStateIdle.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
FsmComponent::FsmComponent(Actor& parent) 
	: IComponent(EComponentType::Fsm, parent, EComponentUpdate::kYes),
	_actor(parent),
	_stateEnterTime(0),
	_stateElipsedAdd(0)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
FsmComponent::~FsmComponent()
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void FsmComponent::initialize()
{
	switch (_actor.getType())
	{

	case EActorType::Player:
	{
		auto player = _actor.shared_from_this<Player>();
		ENSURE(player, return);

		_stateMap.insert(std::make_pair(
			EFsmStateType::PlayerStateIdle,
			std::make_shared<PlayerStateIdle>(*this, *player, EFsmStateType::PlayerStateIdle)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::PlayerStateCtrledEnvObj,
			std::make_shared<PlayerStateCtrledEnvObj>(*this, *player, EFsmStateType::PlayerStateCtrledEnvObj)));
	}
	break;
	case EActorType::Npc:
	{
		auto npc = _actor.shared_from_this<Npc>();
		ENSURE(npc, return);

		_stateMap.insert(std::make_pair(
			EFsmStateType::NpcStateSpawn,
			std::make_shared<NpcStateSpawn>(*this, *npc, EFsmStateType::NpcStateSpawn)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::NpcStateIdle,
			std::make_shared<NpcStateIdle>(*this, *npc, EFsmStateType::NpcStateIdle)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::NpcStateChase,
			std::make_shared<NpcStateChase>(*this, *npc, EFsmStateType::NpcStateChase)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::NpcStateWaiting,
			std::make_shared<NpcStateWaiting>(*this, *npc, EFsmStateType::NpcStateWaiting)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::NpcStateReturn,
			std::make_shared<NpcStateReturn>(*this, *npc, EFsmStateType::NpcStateReturn)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::NpcStateRoaming,
			std::make_shared<NpcStateRoaming>(*this, *npc, EFsmStateType::NpcStateRoaming)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::NpcStateMoveTo,
			std::make_shared<NpcStateMoveTo>(*this, *npc, EFsmStateType::NpcStateMoveTo)));
	}
	break;

	case EActorType::EnvObj:
	{
		auto envObj = _actor.shared_from_this<EnvObj>();
		ENSURE(envObj, return);

		_stateMap.insert(std::make_pair(
			EFsmStateType::EnvObjStateSpawn,
			std::make_shared< EnvObjStateSpawn >(*this, *envObj, EFsmStateType::EnvObjStateSpawn)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::EnvObjStateActivate,
			std::make_shared< EnvObjStateActivate >(*this, *envObj, EFsmStateType::EnvObjStateActivate)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::EnvObjStateDeactivate,
			std::make_shared< EnvObjStateDeactivate >(*this, *envObj, EFsmStateType::EnvObjStateDeactivate)));
		_stateMap.insert(std::make_pair(
			EFsmStateType::EnvObjStateDespawn,
			std::make_shared< EnvObjStateDespawn >(*this, *envObj, EFsmStateType::EnvObjStateDespawn)));
	}
	break;

	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		정리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void FsmComponent::finalize()
{
	_stateMap.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	업데이트한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void FsmComponent::update(int64_t curTime)
{
	if (_stateElipsedAdd)
		curTime += _stateElipsedAdd;

	if (_curState)
		_curState->onUpdate(curTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  스테이트를 변경한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void FsmComponent::_chaseState(EFsmStateType type, const StateInit* init)
{
	_prevState = _curState;
	_curState = nullptr;

	auto iter = _stateMap.find(type);
	if (iter != _stateMap.end())
	{
		_curState = iter->second;
	}
	else
	{
		WARN_LOG("state type is invalid [type:%d]", (int32_t)type);
	}

	if (_prevState)
		_prevState->onExit(_curState.get());

	if (_curState)
	{
		_actor.setState(type);
		_stateElipsedAdd = 0;
		_stateEnterTime = Core::Time::GetCurTimeMSec();
		_curState->onEnter(_prevState.get(), init);
		_curState->onEnterPost();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	상태를 변경한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void FsmComponent::changeState(EFsmStateType type, const StateInit* init)
{
	_chaseState(type, init);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	현재스테이트에서 작업을 실행한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void FsmComponent::doTask(std::function<void(IState*)> task)
{
	if (_curState)
	{
		if (task)
			task(_curState.get());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스테이트 경과시간을 구한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
int32_t FsmComponent::getElapsedTime(int64_t curTime) const
{
	if (_stateElipsedAdd)
		curTime += _stateElipsedAdd;

	return (int32_t)(curTime - _stateEnterTime);
}
