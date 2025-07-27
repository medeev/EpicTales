////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		동적생성되는 액터들에 대한 팩토리 클래스
///
///	@ date		2024-3-17
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "ActorTypes.h"

class ProjectileInfo;
class NpcInfo;
class NpcSpawnInfo;
class EnvObjInfo;
class EnvObjSpawnInfo;
class GroundObjInfo;
class SkillInfo;
class CompanionInfo;
class WorldInfo;
class ItemInfo;

class NpcSpawner;
class ActorFactory
{
public:
	/// Npc를 생성한다.
	static NpcPtr CreateNpc( 
		const NpcInfo& npcInfo,
		const Core::Transform& transform,
		const WorldInfo& worldInfo);

	/// 환경오브젝트를 생성한다. 
	static EnvObjPtr CreateEnvObj(
		const EnvObjInfo& envObjInfo,
		const Core::Transform& transform);
};