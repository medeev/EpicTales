

#include "Pch.h"
#include "EnvObjSpawner.h"

#include <Core/Util/IdFactory.h>
#include <Data/Info/WorldInfo.h>
#include <Data/WorldEntityData/WorldEntityData.h>

#include "Actor/ActorFactory.h"
#include "Actor/EnvObj.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
EnvObjSpawner::EnvObjSpawner(World& world, const EnvObjSpawnData& info, const EnvObjInfo& envObjInfo)
	:
	_world(world),
	_spawnInfo(info),
	_envObjInfo(envObjInfo),
	_spawnedCount(0)
{
	init();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void EnvObjSpawner::init()
{
	if (_spawnInfo._spawnCount <= 0)
	{
		WARN_LOG("MapSpawnCount is invalid [Info: %d, SpawnCount:%d]",
			_spawnInfo.getInfoId(), _spawnInfo._spawnCount);
		return;
	}
	std::vector<Core::Transform> outTransforms;

	for (const auto& transform : _spawnInfo._spawnTransforms)
	{
		auto location = transform.location();
		Vector outLocation;

		if (!_world.getInfo().getProjectLocation(location, outLocation))
		{
			WARN_LOG("spawn navi project invalid [WorldInfo: %u, EnvObjInfo: %u, location:%s]",
				_world.getInfoId(), _spawnInfo.getInfoId(), location.getStr().c_str());
			continue;
		}

		outTransforms.push_back(transform);
	}

	auto timeValue = _world.getTime();
	for (const auto& trnsform : outTransforms)
		_spawnTransformPool.push_back(trnsform);

	for (int32_t i = 0; i < _spawnInfo.getSpawnCount(); ++i)
		addSpawnTimer(timeValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		갱신한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void EnvObjSpawner::update(int64_t curTime)
{
	if (_spawnTimes.empty())
		return;

	int64_t spawnTime = _spawnTimes.front();
	while (!_spawnTimes.empty() && curTime >= spawnTime)
	{
		if (_doSpawn())
		{
			_spawnTimes.pop_front();

			if (!_spawnTimes.empty())
				spawnTime = _spawnTimes.front();
		}
		else {
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스폰시킨다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool EnvObjSpawner::_doSpawn()
{
	if (_spawnInfo.getSpawnCount() <= _spawnedCount)
		return false;

	if (_spawnTransformPool.empty())
		return false;

	auto transform = _spawnTransformPool.front();
	auto envObj = ActorFactory::CreateEnvObj(_envObjInfo, transform);
	if (!envObj)
		return false;

	_spawnTransformPool.pop_front();

	envObj->setSpawner(shared_from_this());
	envObj->setLifeTime(_world.getTime() + _envObjInfo.getLifeTime());
	envObj->enterWorld(_world);

	_spawnedEnvObjIds.insert(envObj->getId());
	_spawnedCount++;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스폰타임을 추가한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void EnvObjSpawner::addSpawnTimer(int64_t timeValue)
{
	_spawnTimes.push_back(timeValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	액터가 삭제될때 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void EnvObjSpawner::removeSpawnTimer(const EnvObj& envObj)
{
	--_spawnedCount;
	_spawnedEnvObjIds.erase(envObj.getId());
	_spawnTransformPool.push_back(envObj.getSpawnTransform());

	if (_spawnInfo._respawnInterval)
		addSpawnTimer(_world.getTime() + _spawnInfo._respawnInterval);
}
