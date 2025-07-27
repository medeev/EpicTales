////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		월드가젯스폰
///
///	@ date		2024-3-24
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Data/WorldEntityData/WorldEntityTypes.h>

#include "Actor/ActorTypes.h"
#include "Component/IComponent.h"
#include "Logic/WorldEntity/EnvObjSpawner.h"
#include "SpawnerTypes.h"

class World;
class EnvObjSpawnComponent
	:
	public IComponent
{
	/// 스포너 리스트 타입 정의
	typedef std::list< EnvObjSpawnerPtr > EnvObjSpawners;

	/// 그룹 식별자별 스포너 리스트 목록 타입 정의
	typedef std::unordered_map < uint32_t, EnvObjSpawners >	EnvObjSpawnerByGroupIdList;

private:
	World& _world; ///< 부모 엔티티
	EnvObjSpawners _spawners; ///< 스폰목록
	int64_t _nextUpdate; ///< 다음업데이트

public:
	/// 생성자
	EnvObjSpawnComponent(World& world);

	/// 소멸자
	virtual ~EnvObjSpawnComponent();

	static EComponentType GetComponentType() {
		return EComponentType::EnvObjSpawn;
	}

	/// 스폰그룹을 특정그룹으로 리셋한다.
	void resetSpawnGroup(const std::vector<EnvObjSpawnDataPtr>& envObjSpawnDatas);

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// 정리 한다
	virtual void finalize() override;

	/// 갱신 한다
	virtual void update(int64_t curTime) override;

};
