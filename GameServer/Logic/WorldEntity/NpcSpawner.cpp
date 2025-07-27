

#include "Pch.h"
#include "NpcSpawner.h"

#include <Core/Util/IdFactory.h>
#include <Data/Info/NpcInfo.h>
#include <Protocol/Struct/PktNpc.h>

#include "Actor/ActorFactory.h"
#include "Actor/Npc.h"
#include "Room/World.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcSpawner::NpcSpawner(World& world, const NpcSpawnData& info, const NpcInfo& npcInfo)
	:
	_world(world),
	_npcSpawnInfo(info),
	_npcInfo(npcInfo),
	_spawnedCount(0)
{
	init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcSpawner::~NpcSpawner()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcSpawner::init()
{
	if (_npcSpawnInfo._spawnCount <= 0)
	{
		WARN_LOG("MapSpawnCount is invalid [WorldInfo: %u, NpcInfo: %u, MapSpawnCount:%d]",
			_world.getInfoId(), _npcSpawnInfo._npcInfoId, _npcSpawnInfo._spawnCount);
		return;
	}

	std::vector<Core::Transform> outTransforms;

	for (const auto& transform : _npcSpawnInfo._spawnTransforms)
	{
		auto location = transform.location();
		location.z -= _npcInfo.getCapsuleHalfHeight();

		Vector outLocation;

		if (!_world.getInfo().getProjectLocation(location, outLocation))
		{
			WARN_LOG("spawn navi project invalid [WorldInfo: %u, NpcInfo: %u, location[%s]]",
				_world.getInfoId(), _npcSpawnInfo._npcInfoId, location.getStr().c_str());
			continue;
		}

		outTransforms.push_back(transform);
	}

	auto timeValue = _world.getTime();
	for (const auto& trnsform : outTransforms)
		_spawnTransformPool.push_back(trnsform);

	for (int32_t i = 0; i < _npcSpawnInfo._spawnCount; ++i)
		addSpawnTimer(timeValue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		갱신한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcSpawner::update(int64_t curTime)
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
/// @brief	스폰시키고 성공여부를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcSpawner::_doSpawn()
{
	if (_npcSpawnInfo._spawnCount <= (int32_t)_spawnedCount)
		return false;

	if (_spawnTransformPool.empty())
		return false;

	auto transform = _spawnTransformPool.front();
	auto npc = ActorFactory::CreateNpc(
		_npcInfo, transform, _world.getInfo());
	if (!npc)
		return false;

	_spawnTransformPool.pop_front();

	npc->setSpawner(shared_from_this());
	npc->enterWorld(_world);

	_spawnedNpcs.insert({ npc->getId(), npc });
	_spawnedCount++;

	PktNpcSpawnNotify notify;

	PktNpcInfo pktInfo;
	npc->exportTo(pktInfo);
	notify.getNpcInfos().emplace_back(std::move(pktInfo));

	_world.sendToNear(*npc, notify);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	대상을 스폰타이머에서 삭제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcSpawner::removeSpawnTimer(const Npc& npc)
{
	auto iter = _spawnedNpcs.find(npc.getId());
	if (iter == _spawnedNpcs.end())
		return;

	--_spawnedCount;
	_spawnedNpcs.erase(iter);
	_spawnTransformPool.push_back(npc.getSpawnTransform());

	addSpawnTimer(_world.getTime() + _npcSpawnInfo._respawnInterval);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	스폰타임을 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcSpawner::addSpawnTimer(int64_t timeValue)
{
	_spawnTimes.push_back(timeValue);
}

