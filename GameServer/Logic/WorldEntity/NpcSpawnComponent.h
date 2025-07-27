////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		월드Npc스폰 컴포넌트
///
///	@ date		2024-3-15
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Data/WorldEntityData/WorldEntityTypes.h>

#include "Actor/ActorTypes.h"
#include "Component/IComponent.h"
#include "Logic/WorldEntity/NpcSpawner.h"
#include "SpawnerTypes.h"

class World;
class NpcSpawnComponent
	:
	public IComponent
{
	/// 스포너 리스트 타입 정의
	typedef std::list<NpcSpawnerPtr> NpcSpawnerList;
public:
	static EComponentType GetComponentType() {
		return EComponentType::NpcSpawn;
	}

private:
	World& _world; ///< 월드정보
	NpcSpawnerList _spawners; ///< 스폰목록
	int64_t _nextUpdate; ///< 다음업데이트
public:
	/// 생성자
	NpcSpawnComponent(World& world);

	/// 소멸자
	virtual ~NpcSpawnComponent();

	/// 현재 스폰컴포넌트로 생성된 Npc를 모두 죽인다
	void removeAllSpawnedNpc();

	/// Npc스폰그룹을 특정그룹으로 리셋한다.
	void resetSpawnGroup(const std::vector<NpcSpawnDataPtr>&);

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// 정리 한다
	virtual void finalize() override;

	/// 갱신 한다
	virtual void update(int64_t curTime) override;
};
