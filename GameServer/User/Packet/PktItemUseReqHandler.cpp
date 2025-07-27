////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktItemUseReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktItemUseReqHandler.h"

#include <Data/Info/ItemInfo.h>

#include "Logic/Item/InventoryComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktItemUseReqHandler::onHandler(User& user, PktItemUseReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktItemUseReqHandler::_onHandler(User& user, PktItemUseReq& req, std::shared_ptr<Ack>& ack)
{
	auto player = user.getPlayer();
	if (!player)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	auto world = player->getWorld();
	if (!world)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	if (req.getItemIdCounts().empty() && req.getItemInfoIdCounts().empty())
	{
		ack->setResult(EResultCode::ParamError);
		user.send(*ack);
		return;
	}

	for (const auto& itemInfoIdCount : req.getItemInfoIdCounts())
	{
		auto itemInfoId = itemInfoIdCount.first;
		if (itemInfoId == 0)
		{
			ack->setResult(EResultCode::ParamError);
			user.send(*ack);
			return;
		}
		auto count = itemInfoIdCount.second;
		if (count <= 0)
		{
			ack->setResult(EResultCode::ParamError);
			user.send(*ack);
			return;
		}

	}
	for (const auto& itemIdCount : req.getItemIdCounts())
	{
		auto itemId = itemIdCount.first;
		if (itemId == 0)
		{
			ack->setResult(EResultCode::ParamError);
			user.send(*ack);
			return;
		}
		auto count = itemIdCount.second;
		if (count <= 0)
		{
			ack->setResult(EResultCode::ParamError);
			user.send(*ack);
			return;
		}
	}

	std::vector<std::pair<const ItemInfo*, int32_t>> itemInfoCounts;
	PktItemChangeNotify notify;

	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	auto& inventory = player->getInvenComponent();

	for (const auto& itemIdCount : req.getItemIdCounts())
	{
		auto item = inventory.findItem(
			itemIdCount.first);

		if (!item)
		{
			ack->setResult(EResultCode::NotExistItemById);
			user.send(*ack);
			return;
		}

		if (item->getAccum() < itemIdCount.second)
		{
			ack->setResult(EResultCode::ParamError);
			user.send(*ack);
			return;
		}

		if (!inventory.deleteItem(*cacheTx, item, (int32_t)itemIdCount.second, &notify.getChangedItemData()))
		{
			ack->setResult(EResultCode::ParamError);
			user.send(*ack);
			return;
		}

		itemInfoCounts.emplace_back(&item->getInfo(), itemIdCount.second);
	}

	for (const auto& itemInfoIdCount : req.getItemInfoIdCounts())
	{
		uint32_t itemInfoId = itemInfoIdCount.first;
		auto count = itemInfoIdCount.second;

		ItemInfoPtr itemInfo(itemInfoId);
		if (!itemInfo)
		{
			ack->setResult(EResultCode::NotExistItemByInfoId);
			user.send(*ack);
			return;
		}

		if (!inventory.deleteItem(*cacheTx, itemInfoId, count, &notify.getChangedItemData()))
		{
			ack->setResult(EResultCode::ParamError);
			user.send(*ack);
			return;
		}

		itemInfoCounts.emplace_back(itemInfo, count);
	}

	for (auto itemInfoPair : itemInfoCounts)
	{
		auto itemInfo = itemInfoPair.first;
		auto count = itemInfoPair.second;

		
	}

	cacheTx->ifFailed(*player, [player, ack]()
		{
			ack->setResult(EResultCode::DBError);
			player->send(*ack);
		});

	cacheTx->ifSucceed(*player, [player, itemInfoCounts, notify, ack]()
		{
			player->send(*ack);
			player->send(notify);
		});

	cacheTx->addBusyUpdater(player->createBusyUpdater());
	cacheTx->run();
}


