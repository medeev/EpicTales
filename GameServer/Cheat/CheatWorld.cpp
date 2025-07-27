

#include "Pch.h"
#include "CheatWorld.h"

#include <Data/Info/WorldInfo.h>
#include <Data/WorldEntityData/WorldEntityData.h>
#include <Protocol/Struct/PktRoom.h>

#include "Logic/PlayerManager.h"
#include "Logic/WorldEntity/NpcSpawnComponent.h"
#include "Protocol/PktCommon.h"
#include "Room/World.h"
#include "Room/WorldManager.h"
#include "User/Packet/PktWorldMoveStartReqHandler.h"
#include "User/UserManager.h"

static CheatWorld s_instanceCheatWorld;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatWorld::CheatWorld()
	:
	Cheat("world")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatWorld::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.empty())
		return;

	if (params[0] == "info")
	{
		_printInfo(player, params);
		return;
	}
	else if (params[0] == "infoall")
	{
		WorldManager::Instance().printInfo();
		return;
	}
	else if (params[0] == "path")
	{
		_path(player, params);
		return;
	}
	else if (params[0] == "cell")
	{
		_cell(player, params);
		return;
	}
	else if (params[0] == "moveto")
	{
		_moveTo(player, params);
	}
	else if (params[0] == "randomlocation")
	{
		_randomLocation(player, params);
	}
	else if (params[0] == "location")
	{
		_location(player, params);
	}
	else if (params[0] == "project")
	{
		_project(player, params);
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  정보를 출력한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatWorld::_printInfo(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	const int32_t scrMsgKey = 7777;

	if (auto world = player->getWorld())
	{
		const auto& worldInfo = world->getInfo();

		auto worldIdStr = Core::StringUtil::ImplFormat("id:%llu", world->getId());
		auto worldInfoStr = Core::StringUtil::ImplFormat("infoId:%u", worldInfo.getId());
		auto worldNameStr = Core::StringUtil::ImplFormat("name:%s", worldInfo.getName().c_str());
		auto worldplayerCountStr = Core::StringUtil::ImplFormat("room playerCnt:%d", world->getPlayerCount());
		auto userCountStr = Core::StringUtil::ImplFormat("server userCnt:%zd", UserManager::Instance().size());
		auto playerCountStr = Core::StringUtil::ImplFormat("server playerCnt:%zd", PlayerManager::Instance().size());

		INFO_LOG("=========================================================");
		INFO_LOG(worldIdStr.c_str());
		INFO_LOG(worldInfoStr.c_str());
		INFO_LOG(worldNameStr.c_str());
		INFO_LOG(worldplayerCountStr.c_str());
		INFO_LOG(userCountStr.c_str());
		INFO_LOG(playerCountStr.c_str());

		PktShapeNotify notify;
		notify.getScrMsgList().emplace_back(scrMsgKey + 0, worldIdStr, PktColorTurquoise);
		notify.getScrMsgList().emplace_back(scrMsgKey + 1, worldInfoStr, PktColorTurquoise);
		notify.getScrMsgList().emplace_back(scrMsgKey + 2, worldNameStr, PktColorTurquoise);
		notify.getScrMsgList().emplace_back(scrMsgKey + 3, worldplayerCountStr, PktColorTurquoise);
		notify.getScrMsgList().emplace_back(scrMsgKey + 4, userCountStr, PktColorTurquoise);
		notify.getScrMsgList().emplace_back(scrMsgKey + 4, playerCountStr, PktColorTurquoise);

		player->send(notify);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  위치를 출력한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatWorld::_location(PlayerPtr player, const Core::StringList& params) const
{
	if (params.size() != 2)
		return;

	Core::StringList toStr = Core::StringUtil::Split(params[1], ",");
	if (toStr.size() != 3)
		return;

	Vector location(
		Core::TypeConv::ToSingle(toStr[0]),
		Core::TypeConv::ToSingle(toStr[1]),
		Core::TypeConv::ToSingle(toStr[2]));

	PktShapeNotify notify;

	notify.getSphereList().emplace_back(
		location,
		90.f, 5.f, 1.f, PktColorOrange);
	notify.getTextList().emplace_back(
		location,
		STRINGUTIL_FORMAT("%d,%d,%d", (int32_t)location.x, (int32_t)location.y, (int32_t)location.z), 5.f, 1.f, PktColorOrange);

	player->send(notify);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  위치를 출력한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatWorld::_project(PlayerPtr player, const Core::StringList& params) const
{
	if (params.size() != 2)
		return;

	Core::StringList toStr = Core::StringUtil::Split(params[1], ",");
	if (toStr.size() != 3)
		return;

	Vector locationIn(
		Core::TypeConv::ToSingle(toStr[0]),
		Core::TypeConv::ToSingle(toStr[1]),
		Core::TypeConv::ToSingle(toStr[2]));

	auto world = player->getWorld();
	if (!world)
		return;

	Vector location;
	if (world->getInfo().getProjectLocation(locationIn, location))
	{
		PktShapeNotify notify;

		notify.getSphereList().emplace_back(
			location,
			90.f, 5.f, 1.f, PktColorOrange);
		notify.getTextList().emplace_back(
			location,
			STRINGUTIL_FORMAT("%d,%d,%d", (int32_t)location.x, (int32_t)location.y, (int32_t)location.z), 5.f, 1.f, PktColorOrange);

		player->send(notify);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  셀정보를 출력한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatWorld::_cell(PlayerPtr player, const Core::StringList& params) const
{
	auto world = player->getWorld();
	if (!world)
		return;

	Vector leftTop;
	if (!world->getCellManager().getCellLeftTop(player->getCellNo(), leftTop))
		return;

	auto cellSize = world->getCellManager().getOneCellSize();
	leftTop.z = player->getLocation().z;

	auto rightTop = leftTop;
	rightTop.x += cellSize;

	auto rightBottom = rightTop;
	rightBottom.y += cellSize;

	auto leftBottom = rightBottom;
	leftBottom.x -= cellSize;

	PktShapeNotify notify;
	notify.getLineList().emplace_back(
		leftTop,
		rightTop,
		60.f/*duration*/,
		10.f,
		PktColorBlue,
		false);
	notify.getLineList().emplace_back(
		rightTop,
		rightBottom,
		60.f/*duration*/,
		10.f,
		PktColorBlue,
		false);
	notify.getLineList().emplace_back(
		rightBottom,
		leftBottom,
		60.f/*duration*/,
		10.f,
		PktColorBlue,
		false);
	notify.getLineList().emplace_back(
		leftBottom,
		leftTop,
		60.f/*duration*/,
		10.f,
		PktColorBlue,
		false);

	player->send(notify);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  특정 위치로의 Path를 출력해본다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatWorld::_path(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 2)
		return;

	Core::StringList toStr = Core::StringUtil::Split(params[1], ",");
	if (toStr.size() != 3)
		return;

	Vector from(
		Core::TypeConv::ToSingle(toStr[0]),
		Core::TypeConv::ToSingle(toStr[1]),
		Core::TypeConv::ToSingle(toStr[2]));


	if (auto world = player->getWorld())
	{
		auto to = player->getLocation();
		/*from.x = -2207;
		from.y = 2170;
		from.z = 180;

		to.x = -2800;
		to.y = 2800;
		to.z = 180;*/

		PathFindingResult pathFind;
		world->getInfo().findPath(from, to, pathFind);
		const auto& naviPaths = pathFind.getPaths();

		Vector prev;
		PktShapeNotify notify;

		int32_t i = 0;
		for (const auto& path : naviPaths)
		{
			if (i == 0)
			{
			}
			else
			{
				notify.getLineList().emplace_back(
					prev, path, 10.f/*duration*/, 3.f, PktColorBlue, true);
			}

			prev = path;
			i++;
		}

		player->send(notify);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스킬컬리젼을 보이게한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatWorld::_moveTo(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.size() < 2)
		return;

	auto value = Core::TypeConv::ToInteger(params[1]);
	WorldInfoPtr worldInfo(value);
	ENSURE(worldInfo, return);

	auto nextWorld = WorldManager::Instance().acquireWorld(
		worldInfo->getId(), false, 
		[worldInfo](RoomThread& thd)
		{
			WorldPtr world = std::make_shared<World>(thd, *worldInfo);
			return world;
		});

	if (worldInfo->getIsVillage())
	{
		if (auto spot = worldInfo->getRandomPlayerStart())
		{
			World::DoWorldMoveStart(player, nextWorld, spot->getLocation());
		}
		else
		{
			World::DoWorldMoveStart(player, nextWorld, Vector(1000, 1800, 300));
		}

		
	}
	else
	{
		World::DoWorldMoveStart(player, nextWorld, Vector(0, 0, 300));
	}
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  월드상에 랜덤한 위치를 갯수만큼 찍는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatWorld::_randomLocation(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	auto world = player->getWorld();
	if (!world)
		return;

	if (params.size() < 2)
		return;

	auto value = Core::TypeConv::ToInteger(params[1]);

	PktShapeNotify notify;

	std::vector<Vector> outLocations;
	if (!world->getInfo().getRandomLocation(value * 2, outLocations))
	{
		return;
	}

	for (auto& location : outLocations)
	{
		notify.getSphereList().emplace_back(
			location,
			90.f, 5.f, 1.f, PktColorBlue);
		notify.getTextList().emplace_back(
			location,
			STRINGUTIL_FORMAT("%d,%d,%d", (int32_t)location.x, (int32_t)location.y, (int32_t)location.z), 5.f, 1.f, PktColorBlue);
	}

	player->send(notify);
}
