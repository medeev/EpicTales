

#include "Pch.h"
#include "CheatEnvObj.h"

#include <Data/Info/WorldInfo.h>
#include <Data/WorldEntityData/WorldEntityData.h>
#include <Protocol/PktCommon.h>
#include <Protocol/Struct/PktShape.h>

#include "Actor/ActorFactory.h"
#include "Actor/ActorTypes.h"
#include "Actor/EnvObj.h"
#include "Actor/Player.h"
#include "Logic/Fsm/FsmComponent.h"
#include "Logic/WorldEntity/EnvObjSpawnComponent.h"
#include "Room/World.h"

static CheatEnvObj s_instanceCheatEnvObj;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatEnvObj::CheatEnvObj()
	:
	Cheat("envobj")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatEnvObj::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.empty())
		return;

	if (params[0] == "ctrl")
	{
		_ctrlEnvObj(player, params);
	}

	if (params[0] == "state")
	{
		_changeState(player, params);
	}

	if (params[0] == "debug")
	{
		_showDebugInfo(player, params);
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	반경 300 안에 가장 가까운 가젯을 컨트롤한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatEnvObj::_ctrlEnvObj(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	auto world = player->getWorld();
	ENSURE(world, return);

	float dist = Core::Numeric::MaxSingle;
	ActorPtr targetActor;

	world->forEachActor(player->getCellNo(),
		[player, &dist, &targetActor](const ActorPtr& actor)
		{
			if (actor->getType() != EActorType::EnvObj)
				return;

			auto targetDist = actor->getDistanceSqr(*player);
			if (targetDist > 300 * 300)
				return;

			if (dist > targetDist)
			{
				dist = targetDist;
				targetActor = actor;
			}
		}
	);

	if (targetActor)
	{
		if (auto gadget = std::dynamic_pointer_cast<EnvObj> (targetActor))
		{
			auto& comp = player->getFsmComponent();
			comp.changeState(EFsmStateType::PlayerStateCtrledEnvObj);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	가젯 상태를 변경한다.
///
///	@ return 없음
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatEnvObj::_changeState(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	auto world = player->getWorld();
	ENSURE(world, return);

	if (params.size() < 2)
		return;

	auto activate = Core::TypeConv::ToBool(params[1]);

	float dist = Core::Numeric::MaxSingle;
	ActorPtr targetActor;

	world->forEachActor(player->getCellNo(),
		[player, &dist, &targetActor](const ActorPtr& actor)
		{
			if (actor->getType() != EActorType::EnvObj)
				return;

			auto targetDist = actor->getDistanceSqr(*player);
			if (targetDist > 300 * 300)
				return;

			if (dist > targetDist)
			{
				dist = targetDist;
				targetActor = actor;
			}
		}
	);

	if (targetActor)
	{
		if (auto gadget = std::dynamic_pointer_cast<EnvObj> (targetActor))
		{
			if (activate)
			{
				if (gadget->getEnvObjState() == EEnvObjStateType::Activated)
					return;

				gadget->getFsmComponent().changeState(EFsmStateType::EnvObjStateActivate);
			}
			else
			{
				if (gadget->getEnvObjState() == EEnvObjStateType::Deactivated)
					return;
				
				gadget->getFsmComponent().changeState(EFsmStateType::EnvObjStateDeactivate);
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	디버깅 정보를 잠시 출력한다.
///
///	@ return 
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatEnvObj::_showDebugInfo(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	auto world = player->getWorld();
	if (!world)
		return;

	PktShapeNotify notify;
	world->forEachActor(player->getCellNo(), [&notify, world](ActorPtr actor)
		{
			if (actor->getType() != EActorType::EnvObj)
				return;

			auto gadget = actor->shared_from_this<EnvObj>();
			auto infoId = gadget->getInfoId();
			int32_t remainSec = 0;
			if (auto spawnInfo = gadget->getSpawnInfo())
			{
				if (spawnInfo->getChargeTime())
				{
					if (gadget->isFullControlCount())
						remainSec = 0;
					else
						remainSec = (int32_t)((gadget->getNextChargeTime() - world->getTime()) / 1000);
				}
			}

			auto pos = gadget->getLocation();

			PktTextShape text(
				pos,
				Core::StringUtil::ImplFormat("envObjInfoId:%u, remain:%u(%dSec)",
					infoId, gadget->getRemainCtrlCount(), remainSec),
				5.f,
				1.f,
				PktColorCyan);

			notify.getTextList().emplace_back(std::move(text));
		});

	
	player->send(notify);
}
