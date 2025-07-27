

#include "Pch.h"
#include "CheatNpc.h"

#include <Data/Info/CharacterInfo.h>
#include <Data/Info/NpcInfo.h>
#include <Protocol/PktCommon.h>
#include <Protocol/Struct/PktNpc.h>
#include <Protocol/Struct/PktRoom.h>
#include <Protocol/Struct/PktShape.h>

#include "Actor/ActorFactory.h"
#include "Actor/Npc.h"
#include "Logic/WorldEntity/NpcSpawnComponent.h"
#include "Room/World.h"
#include "Util/UtilExporter.h"

static CheatNpc s_instanceCheatNpc;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatNpc::CheatNpc()
	:
	Cheat("npc")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatNpc::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.empty())
		return;

	if (params[0] == "create")
	{
		_create(player, params);
	}
	else if (params[0] == "killrange")
	{
		_killByRange(player, params);
	}
	else if (params[0] == "kill")
	{
		_kill(player, params);
	}
	else if (params[0] == "apply")
	{
		_apply(player, params);
	}
	else if (params[0] == "infos")
	{
		_infos(player, params);
		return;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스킬컬리젼을 보이게한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatNpc::_create(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.size() < 2)
		return;

	auto world = player->getWorld();
	if (!world)
		return;

	auto value = Core::TypeConv::ToInteger(params[1]);
	Vector location(player->getRootLocation());

	auto dir = player->getDirection();
	dir.z = 0;
	dir.normalize();
	location += (dir * 50);
	
	NpcInfoPtr npcInfo(value);
	if (!npcInfo)
		return;

	location.z += npcInfo->getCapsuleHalfHeight();

	auto rotateDir = Vector::FromYaw((float)Core::Numeric::Random(0, 364));
	auto npc = ActorFactory::CreateNpc(
		*npcInfo, Core::Transform( rotateDir.toOrientationRotator(), location), world->getInfo());
	if (!npc)
		return;

	npc->enterWorld(*world);

	PktNpcSpawnNotify notify;

	PktNpcInfo pktInfo;
	npc->exportTo(pktInfo);
	notify.getNpcInfos().emplace_back(std::move(pktInfo));

	world->sendToNear(*npc, notify);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  반경내 Npc를 킬한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatNpc::_killByRange(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.size() < 2)
		return;

	auto world = player->getWorld();
	if (!world)
		return;

	auto radius = Core::TypeConv::ToInteger(params[1]);
	Actors actors;
	world->findNearActors(player, (float)radius, actors, [](const ActorPtr& actor)
		{
			if (actor->getType() == EActorType::Npc)
				return true;
			return false;
		});


	for (auto actor : actors)
	{
		auto npc = actor->shared_from_this<Npc>();
		ENSURE(npc, continue);
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  정보식별자의 Npc를 킬한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatNpc::_kill(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.size() < 2)
		return;

	auto world = player->getWorld();
	if (!world)
		return;

	auto infoId = Core::TypeConv::ToInteger(params[1]);
	ActorMap actors;
	world->forEachNpc(player->getCellNo(), [&actors, infoId](const ActorPtr& actor)
		{
			if (actor->getType() == EActorType::Npc)
			{
				if (auto npc = actor->cast<Npc>())
					if (npc->getInfoId() == infoId)
						actors.insert(std::make_pair(actor->getId(), actor));

				return true;
			}

			return false;
		});


	for (auto actor : actors)
	{
		auto npc = actor.second->shared_from_this<Npc>();
		ENSURE(npc, continue);

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc에게 효과를 부여한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatNpc::_apply(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.size() < 4)
	{
		WARN_LOG("argument error ex) cheat npc apply 7 groggy 1");
		return;
	}
	

	auto world = player->getWorld();
	if (!world)
		return;

	auto infoId = Core::TypeConv::ToInteger(params[1]);
	auto str = params[2];
	auto apply = Core::TypeConv::ToInteger(params[3]);

	Actors actors;
	world->forEachNpc(player->getCellNo(), [&actors, infoId](const ActorPtr& actor)
		{
			if (actor->getType() == EActorType::Npc)
			{
				if (auto npc = actor->cast<Npc>())
					if (npc->getInfoId() == infoId)
						actors.push_back(actor);

				return true;
			}

			return false;
		});


	for (auto actor : actors)
	{
		auto npc = actor->shared_from_this<Npc>();
		if (!npc)
			continue;

		if (str == "unablemove")
		{
			_unmoveable(npc, apply);
		}
		else if (str == "invincible")
		{
			_invincible(npc, apply);
		}
		else if (str == "invisible")
		{
			_invisible(npc, apply);
		}
		else
		{
			WARN_LOG("effect not exist %s", str.c_str());
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  월드에 정보를 표시합니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatNpc::_infos(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	auto world = player->getWorld();
	if (!world)
		return;

	PktShapeNotify notify;
	world->forEachNpc(player->getCellNo(), [&notify](const NpcPtr& npc)
		{
			auto infoId = npc->getInfoId();

			auto pos = npc->getLocation();
			pos.z += 150;
			
			{
				UtilExporter::ExportTo(npc->getCollision(), notify, PktColorYellow);
			}
			{
				pos.z -= 50;
				PktTextShape text(
					pos,
					Core::StringUtil::ImplFormat("npc:%u", infoId),
					3.f,
					1.f,
					PktColorMagenta);

				notify.getTextList().emplace_back(std::move(text));
			}
			{
				pos.z -= 50;
				PktTextShape text(
					pos,
					Core::StringUtil::ImplFormat("id:%llu", npc->getId()),
					3.f,
					1.f,
					PktColorMagenta);

				notify.getTextList().emplace_back(std::move(text));
			}
			{
				std::string strState = FsmTypeToString(npc->getState());
				pos.z -= 50;
				PktTextShape text(
					pos,
					Core::StringUtil::ImplFormat("state:%s", strState.c_str()),
					3.f,
					1.f,
					PktColorMagenta);

				notify.getTextList().emplace_back(std::move(text));
			}
			
			{
				PktSphereShape sphereN(
					npc->getLocation(),
					npc->getInfo().getCognitionDist(),
					3.f,
					1.f,
					PktColorRed);
				notify.getSphereList().emplace_back(std::move(sphereN));
			}
			
		});
	player->send(notify);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatNpc::_invisible(NpcPtr npc, bool isApply) const
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  무적을 부여한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatNpc::_invincible(NpcPtr npc, bool isApply) const
{
	
}

void CheatNpc::_unmoveable(NpcPtr npc, bool isApply) const
{
	
}
