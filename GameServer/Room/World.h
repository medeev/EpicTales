////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		월드
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Logic/WorldEntity/EnvObjSpawnComponent.h"
#include "Logic/WorldEntity/NpcSpawnComponent.h"
#include "Logic/WorldEntity/TriggerComponent.h"
#include "Room.h"

class CollisionUnit;
class WorldInfo;
class World
	:
	public Room
{
	typedef Room super;

private:
	int32_t _channelId;
	const WorldInfo& _worldInfo;

	NpcSpawnComponent _npcSpawnComponent;
	EnvObjSpawnComponent _envObjSpawnComponent;
	TriggerComponent _triggerComponent;

public:
	/// 월드이동 패킷으로 이동되는 공통로직을 작성한다.
	static void DoWorldMoveStart(
		PlayerPtr player, WorldPtr nextWorld, const Vector& location, std::function<void()> succFunc = nullptr);

public:
	/// 생성자
	World(RoomThread& thread, const WorldInfo& worldInfo);

	/// 소멸자
	~World();

public:
	/// 초기화 한다
	virtual void initialize() override;

	/// 해제한다.
	virtual void finalize() override;

	/// 갱신한다
	virtual void update() override;

	TriggerComponent& getTriggerComponent() {
		return _triggerComponent;
	}
	const TriggerComponent& getTriggerComponent() const {
		return _triggerComponent;
	}


	NpcSpawnComponent& getNpcSpawnComponent() {
		return _npcSpawnComponent;
	}
	const NpcSpawnComponent& getNpcSpawnComponent() const {
		return _npcSpawnComponent;
	}

	/// 채널ID를 설정한다.
	void setChannelId(int32_t channelId) {
		_channelId = channelId;
	}
	int32_t getChannelId() const {
		return _channelId;
	}

	PktInfoId getInfoId() const;
	const WorldInfo& getInfo() const {
		return _worldInfo;
	}

	/// 반지름을 이용하여 월드상에 오버랩된 액터를 반환한다.
	void findNearActors(
		const ActorPtr baseActor,
		float radius,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;
	void findNearNpcs(
		const ActorPtr baseActor,
		float radius,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;
	void findNearPlayers(
		const ActorPtr baseActor,
		float radius,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;
	void findNearCharacters(
		const ActorPtr baseActor,
		float radius,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;

	/// 대상 컬리젼과 오버랩된 액터를 반환한다.(제약조건 우선검사 True인)
	bool getCollisionOverlapActors(
		int32_t cellNo,
		Core::Collision::ColliderPtr collider,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;

	/// Raycast로 충돌된 액터를 반환한다.
	bool raycastActor(
		int32_t cellNo,
		const Vector& src,
		const Vector& rayDir,
		float rayLength,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;

	/// 컬리전유닛과 오버랩된 액터들을 가져온다.
	void getUnitOverlapsActors(
		int32_t cellNo,
		CollisionUnit* collisionUnit,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;
	void getUnitOverlapNpcs(
		int32_t cellNo,
		CollisionUnit* collisionUnit,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;
	void getUnitOverlapPlayers(
		int32_t cellNo,
		CollisionUnit* collisionUnit,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;
	void getUnitOverlapCharacters(int32_t cellNo,
		CollisionUnit* collisionUnit,
		Actors& outActors,
		std::function<bool(const ActorPtr&)> constraintsFunc) const;

	/// 픽업아이템을 스폰시킨다.
	void spawnPickupItems(
		Actor& caster,
		Actor& target,
		const std::vector<uint32_t>& itemInfos);

protected:

};
