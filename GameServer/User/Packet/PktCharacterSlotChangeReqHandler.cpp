////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktCharacterSlotChangeReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktCharacterSlotChangeReqHandler.h"

#include <Core/Container/ContainerUtil.h>

#include "Actor/Player.h"
#include "User/User.h"
#include "DB/CacheTx.h"
#include "Logic/Character/CharacterComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktCharacterSlotChangeReqHandler::onHandler(User& user, PktCharacterSlotChangeReq& req)
{
    std::shared_ptr<Ack> ack = std::make_shared<Ack>();
    ack->setReqKey(req.getReqKey());
    _onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktCharacterSlotChangeReqHandler::_onHandler(User& user, PktCharacterSlotChangeReq& req, std::shared_ptr<Ack>& ack)
{
	auto player = user.getPlayer();
	if (!player)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	std::vector<uint64_t> characterIds;
	for (const auto& pktSlot : req.getCharacterSlots())
		if (pktSlot.getCharacterId())
			characterIds.push_back(pktSlot.getCharacterId());

	if (Core::Util::has_duplicates(characterIds))
	{
		ack->setResult(EResultCode::ExistDuplicate);
		user.send(*ack);
		return;
	}

	auto& comp = player->getCharacterComponent();

	std::map<ECharacterSlot, uint64_t> curSlots;
	comp.exportTo(curSlots);
	for (const auto& pktSlot : req.getCharacterSlots())
		curSlots[pktSlot.getSlot()] = pktSlot.getCharacterId();
	characterIds.clear();
	for (const auto& [slot, characterId] : curSlots)
		if (characterId)
			characterIds.push_back(characterId);

	if (Core::Util::has_duplicates(characterIds))
	{
		ack->setResult(EResultCode::ExistDuplicate);
		user.send(*ack);
		return;
	}

	auto cacheTx = std::make_shared<CacheTx>(Caller);

	for (const auto& pktSlot : req.getCharacterSlots())
	{
		if ((int32_t)pktSlot.getSlot() < (int32_t)ESkillSlot::Slot0 || (int32_t)pktSlot.getSlot() >= (int32_t)ESkillSlot::Max)
		{
			ack->setResult(EResultCode::ParamError);
			user.send(*ack);
			return;
		}

		auto resultCode = comp.changeSlot(*cacheTx, pktSlot.getSlot(), pktSlot.getCharacterId());
		if (resultCode != EResultCode::Success)
		{
			ack->setResult(resultCode);
			user.send(*ack);
			return;
		}

	}

	cacheTx->ifFailed(*player, [player, ack]()
		{
			ack->setResult(EResultCode::DBError);
			player->send(*ack);
		});


	cacheTx->ifSucceed(*player, [player, req, ack]()
		{
			ack->setCharacterSlots(req.getCharacterSlots());

			player->send(*ack);
		});

	cacheTx->addBusyUpdater(player->createBusyUpdater());
	cacheTx->run();
}


