

#include "Pch.h"
#include "Cell.h"

#include "Actor/Npc.h"
#include "Actor/Player.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
Cell::Cell()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
Cell::~Cell()
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		초기화 한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Cell::initialize(int32_t cellNo, int32_t widthCount, int32_t heightCount, int32_t width_i, int32_t height_i)
{
	if (cellNo < 0 || widthCount <= 0 || heightCount <= 0)
	{
		WARN_LOG("cellNo < 0 || widthCount <= 0 || heightCount <= 0");
		return false;
	}

	if (width_i * heightCount + height_i != cellNo)
	{
		WARN_LOG("check cell values");
		return false;
	}

	_cellNo = cellNo;
	_width_i = width_i;
	_height_i = height_i;

	_calcNearCells(widthCount, heightCount);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		액터를 추가한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Cell::addActor(Actor& actor)
{
	_addActor(actor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		액터를 제거한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Cell::removeActor(uint64_t actorId)
{
	return _removeActor(actorId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  액터를 순횐한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Cell::foreachActor(const ActorCallback& callback) const
{
	std::vector<ActorPtr> actors;
	{
		for (const auto& pair : _actors)
			actors.push_back(pair.second);
	}
	for (const auto& actor : actors)
		callback(actor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc를 순회힌다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Cell::foreachNpc(const NpcCallback& callback) const
{
	std::vector<NpcPtr> npcs;
	{
		for (const auto& pair : _npcs)
			npcs.push_back(pair.second);
	}
	for (const auto& npc : npcs)
		callback(npc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  플레이어를 순회한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Cell::foreachPlayer(const PlayerCallback& callback) const
{
	std::vector<PlayerPtr> players;
	{
		for (const auto& pair : _players)
			players.push_back(pair.second);
	}
	for (const auto& player : players)
		callback(player);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		인접셀을 계산한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Cell::_calcNearCells(int32_t widthCount, int32_t heightCount)
{
	static const int32_t cellDepth = 1;

	int32_t widthStart = _cellNo / heightCount - cellDepth;
	int32_t heightStart = _cellNo % heightCount - cellDepth;
	int32_t cellCount = cellDepth * 2 + 1;
	for (int32_t width_i = widthStart; width_i < widthStart + cellCount; ++width_i)
	{
		for (int32_t height_i = heightStart; height_i < heightStart + cellCount; ++height_i)
		{
			if (width_i < 0 || width_i >= widthCount || height_i < 0 || height_i >= heightCount)
				continue;

			_nearCells.emplace_back(heightCount * width_i + height_i);
			_nearCellSet.emplace(heightCount * width_i + height_i);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		액터를 추가한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Cell::_addActor(Actor& actor)
{
	if (!actor.getId())
	{
		WARN_LOG("invalid actorId. [id: %lld]", actor.getId());
		return;
	}

	auto actorPtr = actor.shared_from_this();
	if (!_actors.insert({ actorPtr->getId(), actorPtr }).second)
	{
		WARN_LOG("actorList already exist. [id: %lld]", actor.getId());
		return;
	}

	switch (actorPtr->getType())
	{
	case EActorType::Player:
		{
			PlayerPtr player = std::static_pointer_cast<Player>(actorPtr);
			if (!_players.insert({ actorPtr->getId(), player }).second)
				WARN_LOG("playerList already exist. [id: %lld]", actorPtr->getId());
		}
		break;
	case EActorType::Npc:
		{
			NpcPtr npc = std::static_pointer_cast<Npc>(actorPtr);
			if (!_npcs.insert({ actorPtr->getId(), npc }).second)
				WARN_LOG("npcList already exist. [id: %lld]", actorPtr->getId());
		}
		break;
	default:
		break;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		액터를 제거한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Cell::_removeActor(uint64_t actorId)
{
	auto it = _actors.find(actorId);
	if (it == _actors.end())
	{
		WARN_LOG("invalid actorId [id: %lld]", actorId);
		return;
	}

	ActorPtr actor = it->second;
	_actors.erase(it);

	switch (actor->getType())
	{
	case EActorType::Player:
		_players.erase(actor->getId());
		break;
	case EActorType::Npc:
		_npcs.erase(actor->getId());
		break;
	default:
		break;
	}
}
