////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		환경오브젝트스폰
///
///	@ date		2024-3-24
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Protocol/Struct/PktTypes.h>
#include "Actor/ActorTypes.h"

class EnvObjInfo;
class EnvObjSpawnData;
class World;
class EnvObjSpawner
	:
	public std::enable_shared_from_this< EnvObjSpawner >
{
	/// EnvObj 식별자 셋 타입 정의
	typedef std::set< uint64_t > EnvObjIdSet;

	/// EnvObj 스폰 시간 정의
	typedef std::deque< int64_t > SpawnTimeQueue;

	/// EnvObj 식별자 목록 타입 정의
	typedef std::map<PktObjId, EnvObjPtr> EnvObjs;

private:
	World& _world; ///< 월드
	const EnvObjInfo& _envObjInfo; ///< 가젯정보
	const EnvObjSpawnData& _spawnInfo; ///< 스폰정보
	EnvObjIdSet _spawnedEnvObjIds; ///< 스폰한 가젯 식별자 목록
	SpawnTimeQueue _spawnTimes; ///< 스폰타임큐
	int32_t _spawnedCount; ///< 스폰된 숫자
	std::list<Core::Transform> _spawnTransformPool; ///< 스폰 위치풀

public:
	/// 생성자
	EnvObjSpawner(World& world, const EnvObjSpawnData& info, const EnvObjInfo& envObjInfo);

	/// 초기화한다.
	void init();

	/// 갱신 한다
	void update(int64_t curTime);

	/// 스폰정보를 반환한다.
	const EnvObjSpawnData& getInfo() const {
		return _spawnInfo;
	}

	/// 스폰타이머를 추가한다.
	void addSpawnTimer(int64_t timeValue);

	/// 액터가 삭제될때 처리한다.
	void removeSpawnTimer(const EnvObj& gadget);

protected:
	/// 스폰 시킨다.
	bool _doSpawn();
};
