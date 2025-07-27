////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		FsmTypes.cpp
///
///	@ date		2024-7-19
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FsmTypes.h"

std::string FsmTypeToString( EFsmStateType v )
{
	switch ( v )
	{
	case EFsmStateType::StateNone:                 return "StateNone";
	case EFsmStateType::PlayerStateIdle:           return "PlayerStateIdle";
	case EFsmStateType::PlayerStateCtrledEnvObj:   return "PlayerStateCtrledEnvObj";
	case EFsmStateType::NpcStateSpawn:             return "NpcStateSpawn";
	case EFsmStateType::NpcStateIdle:              return "NpcStateIdle";
	case EFsmStateType::NpcStateChase:             return "NpcStateChase";
	case EFsmStateType::NpcStateReturn:            return "NpcStateReturn";
	case EFsmStateType::NpcStateRoaming:           return "NpcStateRoaming";
	}

	return "";
}
