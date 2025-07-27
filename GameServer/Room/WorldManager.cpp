

#include "Pch.h"
#include "WorldManager.h"

#include <Data/Info/WorldInfo.h>

#include "RoomThreadManager.h"
#include "World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화 한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void WorldManager::initialize()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  해제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void WorldManager::finailze()
{
	Core::SharedMutex::WriteLockGuard writeLock(_worldLock);
	_publicChannels.clear();
	_worlds.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		월드를 획득한다
////////////////////////////////////////////////////////////////////////////////////////////////////
WorldPtr WorldManager::acquireWorld(
	int32_t worldInfoId, bool isNew, const CreateFunction& createFunc)
{
	// 일단 임시
	bool isPublic = true;

	if (isPublic && !isNew)
	{
		const int32_t publicChannelMaxPlayerCount = 30;

		Core::SharedMutex::ReadLockGuard autoLock(_worldLock);

		auto iter = _publicChannels.find(worldInfoId);
		if (iter != _publicChannels.end())
		{
			const auto& channels = iter->second;
			WorldPtr minChannelWorld = nullptr;
			for (const auto& channel : channels)
			{
				auto publicWorld = channel.second;

				if (publicWorld->isRoomRemovable())
				{
					WARN_LOG("setRemoveable() is set other sourcecode. only enable deleteWorld [worldId: %llu]", publicWorld->getId());
					continue;
				}

				auto publicWorldPlayerCount =
					publicWorld->getPlayerCount() +
					publicWorld->getEnterReserveCount() -
					publicWorld->getLeaveReserveCount();

				if (publicWorldPlayerCount >= publicChannelMaxPlayerCount)
					continue;

				if (!minChannelWorld)
				{
					minChannelWorld = publicWorld;
				}
				else
				{
					auto minChannelPlayerCount =
						minChannelWorld->getPlayerCount() +
						minChannelWorld->getEnterReserveCount() -
						minChannelWorld->getLeaveReserveCount();

					if (minChannelPlayerCount > publicWorldPlayerCount)
						minChannelWorld = publicWorld;
				}
			}
			if (minChannelWorld)
				return minChannelWorld;
		}
	}

	auto thread = RoomThreadManager::Instance().getIdleThread();
	ENSURE(thread, return nullptr);

	auto world = createFunc(*thread);
	if (!world)
		return nullptr;

	world->initialize();
	{
		Core::SharedMutex::WriteLockGuard writeLock(_worldLock);
		_worlds.insert(std::make_pair(world->getId(), world));

		if (isPublic)
		{
			auto& channels = _publicChannels[worldInfoId];
			auto channelId = (int32_t)channels.size() + 1;
			world->setChannelId(channelId);
			channels[channelId] = world;
		}
	}

	thread->addRoom(world);
	return world;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		월드를 삭제한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void WorldManager::deleteWorld(PktObjId id)
{
	Core::SharedMutex::WriteLockGuard autoLock(_worldLock);

	auto iterW = _worlds.find(id);
	if (iterW == _worlds.end())
	{
		WARN_LOG("not found world. delete failed. [WorldId: %llu]", id);
		return;
	}

	auto world = iterW->second;

	if (world->isRoomRemovable())
	{
		WARN_LOG("aready setRemoveable set. [worldId: %llu]", id);
		return;
	}

	_worlds.erase(iterW);

	INFO_LOG("world delete: %llu", id);

	auto worldInfoId = world->getInfoId();
	auto iterC = _publicChannels.find(worldInfoId);
	if (iterC != _publicChannels.end())
	{
		auto& channelList = iterC->second;
		channelList.erase(world->getChannelId());
		if (channelList.empty())
		{
			INFO_LOG("channel delete. [channelId: %ld]", world->getChannelId());
			_publicChannels.erase(iterC);
		}
	}

	world->setRemovable(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		월드를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
WorldPtr WorldManager::getWorld(PktObjId worldId)
{
	Core::SharedMutex::ReadLockGuard lock(_worldLock);
	
	auto it = _worlds.find(worldId);
	if (it == _worlds.end())
		return nullptr;

	return it->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  모든룸의 액터 카운트 정보를 출력한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void WorldManager::printInfo()
{
	Core::SharedMutex::ReadLockGuard lock(_worldLock);

	for (const auto& worldPair : _worlds)
	{
		const auto world = worldPair.second;
		const auto& worldInfo = world->getInfo();

		auto worldIdStr = Core::StringUtil::ImplFormat("id:%llu", world->getId());
		auto worldInfoStr = Core::StringUtil::ImplFormat("infoId:%u", worldInfo.getId());
		auto worldNameStr = Core::StringUtil::ImplFormat("name:%s", worldInfo.getName().c_str());
		auto playerCountStr = Core::StringUtil::ImplFormat("playerCnt:%d", world->getPlayerCount());
		auto leaveStr = Core::StringUtil::ImplFormat("reserveLeave:%d", world->getLeaveReserveCount());
		auto enterStr = Core::StringUtil::ImplFormat("reserveEnter:%d", world->getEnterReserveCount());

		INFO_LOG("=========================================================");
		INFO_LOG(worldIdStr.c_str());
		INFO_LOG(worldInfoStr.c_str());
		INFO_LOG(worldNameStr.c_str());
		INFO_LOG(playerCountStr.c_str());
		INFO_LOG(leaveStr.c_str());
		INFO_LOG(enterStr.c_str());
	}

	INFO_LOG("=========================================================");
	auto worldsCountStr = Core::StringUtil::ImplFormat("wordCount:%zd", _worlds.size());
	INFO_LOG(worldsCountStr.c_str());
}

