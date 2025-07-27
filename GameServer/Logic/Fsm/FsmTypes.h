////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		스테이트 타입
///
///	@ date		2024-3-21
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <functional>
#include <Protocol/Struct/PktTypes.h>

enum class EFsmStateType
{
	StateNone = -1,

	// For Player
	PlayerStateIdle,
	PlayerStateCtrledEnvObj,
	
	// For Npc
	NpcStateSpawn,
	NpcStateIdle,
	NpcStateChase,
	NpcStateWaiting,
	NpcStateMoveTo,
	NpcStateReturn,
	NpcStateRoaming,

	// For EnvObj
	EnvObjStateSpawn,
	EnvObjStateActivate,
	EnvObjStateDeactivate,
	EnvObjStateDespawn,
};

/// 액션 조건 타입을 문자열로 변환한다.
std::string FsmTypeToString( EFsmStateType v );

class IState;
typedef std::shared_ptr< IState > IStatePtr;

class PlayerStateIdle;
typedef std::shared_ptr< PlayerStateIdle> PlayerStateIdlePtr;

class NpcStateSkill;
typedef std::shared_ptr< NpcStateSkill > NpcStateSkillPtr;

class NpcStateIdle;
typedef std::shared_ptr< NpcStateIdle > NpcStateIdlePtr;

class EnvObjStateActivate;
typedef std::shared_ptr< EnvObjStateActivate > EnvObjStateActivatePtr;

class PlayerStateCtrledEnvObj;
typedef std::shared_ptr< PlayerStateCtrledEnvObj > PlayerStateCtrledEnvObjPtr;

typedef std::unordered_map< int32_t, std::set< PktObjId > > EffectApplys;
