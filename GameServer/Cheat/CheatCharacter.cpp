

#include "Pch.h"
#include "CheatCharacter.h"

#include <Data/Info/CharacterInfo.h>
#include <Protocol/Struct/PktRoom.h>

#include "Actor/Player.h"
#include "Logic/Fsm/FsmStateInit.h"
#include "Logic/PlayerManager.h"
#include "Logic/Character/CharacterComponent.h"
#include "OrmSub/PlayerCharacter.h"
#include "Room/World.h"
#include "User/UserManager.h"

static CheatCharacter s_instanceCheatCharacter;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatCharacter::CheatCharacter()
	:
	Cheat("character")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatCharacter::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.empty())
		return;

	if (params[0] == "addall")
	{
		_addall(player, params);
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  몽당 획득한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatCharacter::_addall(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	auto& comp = player->getCharacterComponent();
	auto emptySlots = comp.findEmptySlots();

	PktCharacterSlotChangeNotify pktChangeSlotNotify;
	PktCharacterAddOrUpdateNotify pktAddOrUpdateNotify;

	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	for (const auto& characterInfo :
		CharacterInfoManager::Instance().getInfos() | std::views::values)
	{
		if (comp.findPlayerCharacter(characterInfo->getId()))
			continue;
		
		auto character = std::make_shared<PlayerCharacter>(*characterInfo);
		auto cacheCharacter = cacheTx->acquireObject(*player, character);

		PktPlayerCharacter pktCharacter;
		if (!comp.insertCharacter(*cacheTx, characterInfo->getId(), pktCharacter))
			continue;

		pktAddOrUpdateNotify.getAddCharacters().emplace_back(pktCharacter);

		if (!emptySlots.empty())
		{
			auto slotType = emptySlots.front();
			if (comp.changeSlot(*cacheTx, slotType, pktCharacter.getCharacterId()) == EResultCode::Success)
			{
				emptySlots.pop_front();

				PktCharacterSlot pktChangeSlot;
				pktChangeSlot.setSlot(slotType);
				pktChangeSlot.setCharacterId(pktCharacter.getCharacterId());

				pktChangeSlotNotify.getCharacterSlots().emplace_back(std::move(pktChangeSlot));
			}
		}
	}

	cacheTx->ifSucceed(*player, [player, pktChangeSlotNotify, pktAddOrUpdateNotify]()
		{
			player->send(pktAddOrUpdateNotify);
			player->send(pktChangeSlotNotify);
		});

	cacheTx->run();
}
