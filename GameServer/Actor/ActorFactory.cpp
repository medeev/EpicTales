
#include "Pch.h"
#include "ActorFactory.h"

#include <Data/Info/NpcInfo.h>

#include "ActorTypes.h"
#include "EnvObj.h"
#include "Logic/Fsm/FsmComponent.h"
#include "Npc.h"
#include "Player.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	Npc를 생성한다
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcPtr ActorFactory::CreateNpc(
	const NpcInfo& npcInfo, const Core::Transform& transform, const WorldInfo& worldInfo)
{
	NpcPtr npc = std::make_shared<Npc>(npcInfo);
	npc->setPtr(npc);
	npc->initialize();
	npc->setSpawnTransform(transform);
	npc->setLocation(transform.location());
	npc->setDirection(transform.rotator().vector());

	ActorStat infoStats(npcInfo);

	return npc;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	환경오브젝트를 생성한다
////////////////////////////////////////////////////////////////////////////////////////////////////
EnvObjPtr ActorFactory::CreateEnvObj( 
	const EnvObjInfo& envObjInfo,
	const Core::Transform& transform)
{
	EnvObjPtr envObj = std::make_shared<EnvObj>( envObjInfo );
	envObj->setPtr( envObj );
	envObj->initialize();
	envObj->setSpawnLocation(transform);
	envObj->setLocation(transform.location());
	envObj->setDirection(transform.rotator().vector());

	return envObj;
}
