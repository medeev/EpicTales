

#include "Pch.h"
#include "CheatPlayer.h"

#include <Protocol/PktCommon.h>
#include <Protocol/Struct/PktRoom.h>
#include <Protocol/Struct/PktShape.h>

#include "Actor/ActorFactory.h"
#include "Actor/Player.h"
#include "Room/World.h"
#include "Util/UtilExporter.h"
#include "Logic/Character/CharacterComponent.h"

static CheatPlayer s_instanceCheatPlayer;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatPlayer::CheatPlayer()
	:
	Cheat("player")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatPlayer::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.empty())
		return;

	if (params[0] == "debug")
	{
		_debug(player, params);
		return;
	}
	else if (params[0] == "info")
	{
		_printInfo(player, params);
	}
	else if (params[0] == "exp")
	{
		_exp(player, params);
	}
	else if (params[0] == "token")
	{
		_token(player, params);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  디버깅용 치트입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatPlayer::_debug(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	auto world = player->getWorld();
	if (!world)
		return;

	if (params.size() != 2)
		return;

	if (params[1] == "state")
	{
		PktShapeNotify notify;
		world->forEachPlayer(player->getCellNo(), [&notify](const PlayerPtr& player)
			{
				std::string strState = FsmTypeToString(player->getState());

				auto pos = player->getLocation();

				PktTextShape text(
					pos,
					strState.c_str(),
					15.f,
					1.f,
					PktColorMagenta);

				notify.getTextList().emplace_back(std::move(text));
			});
		player->send(notify);
	}
	else if (params[1] == "collision")
	{
		PktShapeNotify notify;
		world->forEachPlayer(player->getCellNo(), [&notify](const PlayerPtr& player)
			{
				UtilExporter::ExportTo(player->getCollision(), notify, PktColorYellow);
			});
		player->send(notify);
	}
	else if (params[1] == "id")
	{
		PktShapeNotify notify;
		world->forEachPlayer(player->getCellNo(), [&notify](const PlayerPtr& player)
			{
				auto pos = player->getLocation();

				PktTextShape text(
					pos,
					Core::StringUtil::ImplFormat("id:%llu", player->getId()),
					15.f,
					1.f,
					PktColorMagenta);

				notify.getTextList().emplace_back(std::move(text));
			});
		player->send(notify);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  정보를 출력한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatPlayer::_printInfo(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	const int32_t scrMsgKey = 7778;

	auto strId = Core::StringUtil::ImplFormat("id:%llu", player->getId());
	auto strState = FsmTypeToString(player->getState());
	auto location = player->getLocation();
	auto strlocation = Core::StringUtil::ImplFormat("location: %d, %d, %d", (int32_t)location.x, (int32_t)location.y, (int32_t)location.z);
	auto strSeeingCount = Core::StringUtil::ImplFormat("seeing count: %zd", player->getSeeingCount());

	INFO_LOG("=========================================================");
	INFO_LOG(strId.c_str());
	INFO_LOG(strState.c_str());
	INFO_LOG(strlocation.c_str());
	INFO_LOG(strSeeingCount.c_str());

	PktShapeNotify notify;
	notify.getScrMsgList().emplace_back(scrMsgKey + 0, strId, PktColorTurquoise);
	notify.getScrMsgList().emplace_back(scrMsgKey + 1, strState, PktColorTurquoise);
	notify.getScrMsgList().emplace_back(scrMsgKey + 2, strlocation, PktColorTurquoise);
	notify.getScrMsgList().emplace_back(scrMsgKey + 3, strSeeingCount, PktColorTurquoise);

	player->send(notify);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  경험치를 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatPlayer::_exp(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 3)
		return;

	//auto value = Core::TypeConv::ToInteger(params[1]);

	//CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	//player->getCharacterComponent().increaseExp(*cacheTx, value);
	//cacheTx->run();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  남은 토큰을 본다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatPlayer::_token(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	auto world = player->getWorld();
	if (!world)
		return;

	std::vector<Vector> outLocations;
	player->getTokenPosition(player->getRadius(), false, outLocations);

	if (outLocations.empty())
		return;

	PktShapeNotify notify;

	for (auto& location : outLocations)
	{
		PktSphereShape pkt;
		pkt.setLocation(location);
		pkt.setRadius(15);
		pkt.setDuration(3.f);
		pkt.setThickness(2.f);
		pkt.setColor(PktColorOrange);
		notify.getSphereList().emplace_back(std::move(pkt));
	}
	player->send(notify);

}
