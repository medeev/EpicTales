
#include "Pch.h"
#include "Npc.h"

#include <Core/Collision/Collider.h>
#include <Core/Util/IdFactory.h>
#include <Data/Info/InfoEnumsConv.h>
#include <Data/Info/NpcInfo.h>
#include <Protocol/Struct/PktNpc.h>
#include <Protocol/Struct/PktPlayer.h>
#include <Protocol/Struct/PktRoom.h>

#include "Logic/Fsm/FsmComponent.h"
#include "Logic/Npc/NpcMoveComponent.h"
#include "Logic/NpcAi/NpcAiComponent.h"
#include "Logic/Reward/RewardProcess.h"
#include "Logic/Skill/SkillComponent.h"
#include "Logic/WorldEntity/NpcSpawnComponent.h"
#include "Logic/WorldEntity/NpcSpawner.h"
#include "Room/Room.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
Npc::Npc(const NpcInfo& info)
	:
	super(EActorType::Npc),
	_info(info),
	_npcAIComponent(*this),
	_npcMoveComponent(*this),
	_skillComponent(*this),
	_minSkillDist(0),
	_isCombating(false)
{
	setId(Core::IdFactory::CreateId());
	
	_teamType = info.getTeam();

	Core::Collision::CapsulePtr capsule = std::make_shared<Core::Collision::Capsule>();
	capsule->init((float)(_info.getCapsuleRadius()), (float)(_info.getCapsuleHalfHeight()));

	_collider = capsule;
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화 한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Npc::initialize()
{
	super::initialize();


}

void Npc::finalize()
{

	super::finalize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		정보를 내보낸다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Npc::exportTo(PktSightEnterNotify& dest) const
{
	PktNpcInfo pktInfo;
	exportTo(pktInfo);
	dest.getNpcs().emplace_back(std::move(pktInfo));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		정보를 내보낸다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Npc::exportTo(PktNpcInfo& dest) const
{
	dest.setId(getId());
	dest.setInfoId(_info.getId());

	dest.setLocation(getLocation());
	dest.setFoward(getDirection());

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  Npc정보식별자를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
PktInfoId Npc::getInfoId() const
{
	return _info.getId();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	반지름을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
float Npc::getRadius() const
{
	return (float)(_info.getCapsuleRadius());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	캡슐높이반을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
float Npc::getHalfHeight() const
{
	float halfHeight = (float)_info.getCapsuleHalfHeight();
	if (halfHeight > 250.f)
		WARN_LOG("capsule half height is too big!! [halfHeight:%.f, limit:%.f]", 
			halfHeight, 250.f);
	return halfHeight;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	루트 포지션값을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
Vector Npc::getRootLocation() const
{
	return _location - Vector(0, 0, (float)(_info.getCapsuleHalfHeight()));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  월드에 진입한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Npc::enterWorld(World& world)
{
	world.addActor(shared_from_this());

	_fsmComponent.changeState(
		EFsmStateType::NpcStateSpawn);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Npc::setSpawner(NpcSpawnerPtr spawner)
{
	_spawnerWeak = spawner;
}

bool Npc::getLocationByType(NpcPositionType type, Vector& outLocation)
{
	auto iter = _locationMap.find(type);
	if (iter == _locationMap.end())
	{
		return false;
	}

	outLocation = iter->second;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  방에서 삭제된다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Npc::endPlayPost()
{
	ENSURE(!getRoom(), return);

	if (auto spawnner = getSpawner())
		spawnner->removeSpawnTimer(*this);
	_npcAIComponent.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  전투에 진입하고 나간다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Npc::enterCombat()
{
	if (_isCombating)
		return;

	_isCombating = true;

	setLocationByType(NpcPositionType::CombatStartPos, getLocation());

	getNpcAiComponent().enterCombat();
}

void Npc::exitCombat()
{
	if (!_isCombating)
		return;

	_isCombating = false;

	getNpcAiComponent().exitCombat();
	getNpcMoveComponent().returnToken();
}

