

#include "Pch.h"
#include "World.h"

#include <Data/Collision/Unit/CollisionUnit.h>
#include <Data/Collision/Unit/CollisionUnitBuffer.h>
#include <Data/Info/ItemInfo.h>
#include <Data/Info/WorldInfo.h>
#include <Protocol/Struct/PktRoom.h>

#include "Actor/Actor.h"
#include "Actor/ActorFactory.h"
#include "Actor/ActorTypes.h"
#include "Actor/Player.h"
#include "Logic/WorldEntity/EnvObjSpawnComponent.h"
#include "Logic/WorldEntity/NpcSpawnComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
World::World(RoomThread& thread, const WorldInfo& worldInfo)
	:
	super(thread), _channelId(0), _worldInfo(worldInfo),
	_npcSpawnComponent(*this),
	_envObjSpawnComponent(*this),
	_triggerComponent(*this)
{
	_cellManager.initialize(3000, _worldInfo.getNaviMeshRect());
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
World::~World()
{
	INFO_LOG("world delete [%llu]", getId());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  월드이동 패킷으로 이동되는 공통로직을 작성한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::DoWorldMoveStart(
	PlayerPtr player, WorldPtr nextWorld, const Vector& location, std::function<void()> succFunc)
{
	ENSURE(player, return);
	ENSURE(nextWorld, return);

	player->setWorldMoveLoading(true);

	auto busy = player->createBusyUpdater();
	player->leaveWorld([nextWorld, location, busy, succFunc](Player& player)
		{
			player.setLocation(location);
			player.enterWorld(nextWorld, [busy, succFunc](WorldPtr world, PlayerPtr player)
				{
					if (succFunc != nullptr)
						succFunc();

					PktWorldMoveStartNotify notify;
					notify.setWorldInfoId(world->getInfo().getId());
					notify.setTargetWorldId(world->getId());
					notify.setLocation(player->getLocation());
					player->send(notify);
				});
		});
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화 한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::initialize()
{
	INFO_LOG("world init [%llu]", getId());
	super::initialize();

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  해제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::finalize()
{

	super::finalize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		갱신한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::update()
{
	super::update();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  월드정보 식별자를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
PktInfoId World::getInfoId() const
{
	return _worldInfo.getId();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	 대상 반지름에 오버랩된 액터를 반환한다.(제약조건 우선검사 True인)
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::findNearActors(
	const ActorPtr baseActor,
	float radius,
	Actors& outActors,
	std::function<bool(const ActorPtr&)> constraintsFunc) const
{
	ENSURE(baseActor, return);

	auto logic = [baseActor, radius, constraintsFunc, &outActors](const Actor& actor)
		{
			ENSURE(baseActor, return false);

			if (!constraintsFunc(actor.shared_from_this()))
				return true;

			const auto deltaPos = baseActor->getLocation() - actor.getLocation();

			if (deltaPos.lengthSq() <= radius * radius)
				outActors.push_back(actor.shared_from_this());

			return true;
		};

	forEachActor(
		baseActor->getCellNo(),
		[logic](const ActorPtr& actor)
		{
			if (actor->isWorldMoveLoading())
				return true;

			return logic(*actor);
		}
	);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  대상 반지름에 Npc를 반환한다.(제약조건 우선검사 True인)
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::findNearNpcs(const ActorPtr baseActor, float radius, Actors& outActors, std::function<bool(const ActorPtr&)> constraintsFunc) const
{
	ENSURE(baseActor, return);

	auto logic = [baseActor, radius, constraintsFunc, &outActors](const Actor& actor)
		{
			ENSURE(baseActor, return false);

			if (!constraintsFunc(actor.shared_from_this()))
				return true;

			const auto deltaPos = baseActor->getLocation() - actor.getLocation();

			if (deltaPos.lengthSq() <= radius * radius)
				outActors.push_back(actor.shared_from_this());

			return true;
		};

	forEachNpc(
		baseActor->getCellNo(),
		[logic](const ActorPtr& actor)
		{
			if (actor->isWorldMoveLoading())
				return true;

			return logic(*actor);
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  대상 반지름에 플레이어를 반환한다.(제약조건 우선검사 True인)
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::findNearPlayers(const ActorPtr baseActor, float radius, Actors& outActors, std::function<bool(const ActorPtr&)> constraintsFunc) const
{
	ENSURE(baseActor, return);

	auto logic = [baseActor, radius, constraintsFunc, &outActors](const Actor& actor)
		{
			ENSURE(baseActor, return false);

			if (!constraintsFunc(actor.shared_from_this()))
				return true;

			const auto deltaPos = baseActor->getLocation() - actor.getLocation();

			if (deltaPos.lengthSq() <= radius * radius)
				outActors.push_back(actor.shared_from_this());

			return true;
		};

	forEachPlayer(
		baseActor->getCellNo(),
		[logic](const ActorPtr& actor)
		{
			if (actor->isWorldMoveLoading())
				return true;

			return logic(*actor);
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  대상 반지름에 케릭터를 반환한다.(제약조건 우선검사 True인)
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::findNearCharacters(const ActorPtr baseActor, float radius, Actors& outActors, std::function<bool(const ActorPtr&)> constraintsFunc) const
{
	findNearNpcs(baseActor, radius, outActors, constraintsFunc);
	findNearPlayers(baseActor, radius, outActors, constraintsFunc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	 대상 컬리젼과 오버랩된 액터를 반환한다.(제약조건 우선검사 True인)
////////////////////////////////////////////////////////////////////////////////////////////////////
bool World::getCollisionOverlapActors(
	int32_t cellNo,
	Core::Collision::ColliderPtr collider,
	Actors& outActors,
	std::function< bool(const ActorPtr&) > constraintsFunc) const
{
	auto logic = [collider, constraintsFunc, &outActors](const Actor& actor)
		{
			if (!constraintsFunc(actor.shared_from_this()))
				return true;

			if (Core::Collision::CheckCollision(collider.get(), actor.getCollision().get()))
				outActors.push_back(actor.shared_from_this());

			return true;
		};

	forEachActor(
		cellNo,
		[logic](const ActorPtr& actor)
		{
			if (actor->isWorldMoveLoading())
				return true;

			return logic(*actor);
		});

	return !outActors.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	 대상 컬리전과 월드상에 오버랩된 케릭터형를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::getUnitOverlapCharacters(
	int32_t cellNo, CollisionUnit* collisionUnit, Actors& outActors, std::function<bool(const ActorPtr&)> constraintsFunc) const
{
	getUnitOverlapPlayers(cellNo, collisionUnit, outActors, constraintsFunc);
	getUnitOverlapNpcs(cellNo, collisionUnit, outActors, constraintsFunc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  바닥아이템을 생성한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::spawnPickupItems(
	Actor& caster,
	Actor& target,
	const std::vector<uint32_t>& itemInfoIds)
{
	std::vector<PickupItemPtr> pickupItems;
	for (auto itemInfoId : itemInfoIds)
	{
		ItemInfoPtr itemInfo(itemInfoId);
		ENSURE(itemInfo, continue);

		Vector location;
		getInfo().getRandomLocationPathConnectedInDonut(
			caster.getRootLocation(),
			caster.getRadius(),
			caster.getRadius() + 100,
			location);
		location.z += 10;

	}

	if (!pickupItems.empty())
	{
		
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  충돌되는 액터를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool World::raycastActor(
	int32_t cellNo, 
	const Vector& src, 
	const Vector& rayDir,
	float rayLength, 
	Actors& outActors,
	std::function< bool(const ActorPtr&) > constraintsFunc) const
{
	Core::Collision::SpherePtr sphere = std::make_shared<Core::Collision::Sphere>();
	sphere->init(10.f);

	Core::Transform transform(Core::Rotation::FromIdentity().rotator(), src);

	auto logic = [sphere, transform, rayDir, rayLength, constraintsFunc, &outActors](const Actor& actor)
		{
			if (!constraintsFunc(actor.shared_from_this()))
				return true;

			float outTime;
			Vector outPosition;
			Vector outNormal;
			if (Core::Collision::Raycast(
				actor.getCollision().get(),
				sphere.get(),
				transform, rayDir, rayLength, outTime, outPosition, outNormal))
			{
				outActors.push_back(actor.shared_from_this());
			}

			return true;
		};

	forEachPlayer(
		cellNo,
		[logic](const PlayerPtr& actor)
		{
			if (actor->isWorldMoveLoading())
				return true;

			return logic(*actor);
		});

	forEachNpc(
		cellNo,
		[logic](const NpcPtr& actor)
		{
			return logic(*actor);
		});


	if (outActors.empty())
		return false;

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  컬리전유닛과 오버랩된 액터들을 가져온다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void World::getUnitOverlapsActors(
	int32_t cellNo,
	CollisionUnit* collisionUnit, 
	Actors& outActors,
	std::function<bool(const ActorPtr&)> constraintsFunc) const
{
	if (!collisionUnit)
		return;

	CollisionUnitBuffer buffer;

	auto logic = [&buffer, collisionUnit, constraintsFunc, &outActors](const Actor& actor)
		{
			if (!constraintsFunc(actor.shared_from_this()))
				return true;

			if (auto actorUnit = actor.getCollisionUnit(buffer))
				if (collisionUnit->checkTo(*actorUnit))
					outActors.push_back(actor.shared_from_this());

			return true;
		};

	forEachActor(
		cellNo,
		[logic](const ActorPtr& actor)
		{
			if (actor->isWorldMoveLoading())
				return true;

			return logic(*actor);
		});
}

void World::getUnitOverlapNpcs(
	int32_t cellNo, CollisionUnit* collisionUnit, Actors& outActors, std::function<bool(const ActorPtr&)> constraintsFunc) const
{
	if (!collisionUnit)
		return;

	CollisionUnitBuffer buffer;

	auto logic = [&buffer, collisionUnit, constraintsFunc, &outActors](const Actor& actor)
		{
			auto actorPtr = actor.shared_from_this();
			if (!constraintsFunc(actorPtr))
				return true;

			if (auto actorUnit = actor.getCollisionUnit(buffer))
				if (collisionUnit->checkTo(*actorUnit))
					outActors.push_back(actorPtr);

			return true;
		};

	forEachNpc(
		cellNo,
		[logic](const ActorPtr& actor)
		{
			if (actor->isWorldMoveLoading())
				return true;

			return logic(*actor);
		});
}

void World::getUnitOverlapPlayers(
	int32_t cellNo, CollisionUnit* collisionUnit, Actors& outActors, std::function<bool(const ActorPtr&)> constraintsFunc) const
{
	if (!collisionUnit)
		return;

	CollisionUnitBuffer buffer;

	auto logic = [&buffer, collisionUnit, constraintsFunc, &outActors](const Actor& actor)
		{
			auto actorPtr = actor.shared_from_this();

			if (!constraintsFunc(actorPtr))
				return true;

			if (auto actorUnit = actor.getCollisionUnit(buffer))
				if (collisionUnit->checkTo(*actorUnit))
					outActors.push_back(actorPtr);

			return true;
		};

	forEachPlayer(
		cellNo,
		[logic](const ActorPtr& actor)
		{
			if (actor->isWorldMoveLoading())
				return true;

			return logic(*actor);
		});
}
