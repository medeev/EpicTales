

#include "Pch.h"
#include "CheatItem.h"

#include <Data/Info/ItemInfo.h>
#include <Protocol/Struct/PktItem.h>
#include <Protocol/Struct/PktRoom.h>

#include "Logic/Item/InventoryComponent.h"
#include "Logic/PlayerManager.h"
#include "User/UserManager.h"
#include "Logic/Item/EquipComponent.h"

static CheatItem s_instanceCheatItem;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatItem::CheatItem()
	:
	Cheat("item")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatItem::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.empty())
		return;

	if (params[0] == "add")
	{
		_add(player, params);
		return;
	}
	else if (params[0] == "del")
	{
		_del(player, params);
		return;
	}
	else if (params[0] == "use")
	{
		_use(player, params);
		return;
	}
	else if (params[0] == "equip")
	{
		_equip(player, params);
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  아이템을 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatItem::_add(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 3)
		return;

	auto infoIdStr = params[1];
	auto countStr = params[2];

	auto infoId = Core::TypeConv::ToInteger(infoIdStr);
	auto count = Core::TypeConv::ToInteger(countStr);

	ItemInfoPtr info(infoId);
	if (!info)
	{
		WARN_LOG("iteminfo id is not exist [infoid:%u]", infoId);
		return;
	}

	auto& inven = player->getInvenComponent();

	PktItemChangeNotify notify;
	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	if (!inven.insertItem(*cacheTx, *info, count, &notify.getChangedItemData()))
		return;

	cacheTx->ifSucceed(*player, [player, notify]()
		{
			player->send(notify);
		});
	cacheTx->run();

	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  아이템을 삭제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatItem::_del(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 3)
		return;

	auto infoIdStr = params[1];
	auto countStr = params[2];

	PktInfoId infoId = Core::TypeConv::ToInteger(infoIdStr);
	auto count = Core::TypeConv::ToInteger(countStr);

	auto& inven = player->getInvenComponent();

	PktItemChangeNotify notify;
	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	if (!inven.deleteItem(*cacheTx, infoId, count, &notify.getChangedItemData()))
		return;

	cacheTx->ifSucceed(*player, [player, notify]()
		{
			player->send(notify);
		});
	cacheTx->run();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  인벤토리 아이템을 사용한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatItem::_use(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 3)
		return;

	auto infoIdStr = params[1];
	auto countStr = params[2];

	PktInfoId infoId = Core::TypeConv::ToInteger(infoIdStr);
	ItemInfoPtr info(infoId);
	if (!info)
	{
		WARN_LOG("iteminfo id is not exist [infoid:%u]", infoId);
		return;
	}

	auto count = Core::TypeConv::ToInteger(countStr);

	auto& inven = player->getInvenComponent();

	PktItemChangeNotify notify;
	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	if (!inven.deleteItem(*cacheTx, infoId, count, &notify.getChangedItemData()))
		return;

	std::vector<std::pair<const ItemInfo*, int32_t>> itemInfoCounts;
	itemInfoCounts.emplace_back(std::make_pair(info, count));

	cacheTx->ifSucceed(*player, [player, itemInfoCounts, notify]()
		{
			player->send(notify);

			
		});
	cacheTx->run();
}


void CheatItem::_equip(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 2)
		return;

	auto infoIdStr = params[1];

	PktInfoId infoId = Core::TypeConv::ToInteger(infoIdStr);
	ItemInfoPtr info(infoId);
	if (!info)
	{
		WARN_LOG("iteminfo id is not exist [infoid:%u]", infoId);
		return;
	}

	auto& inven = player->getInvenComponent();
	auto item = inven.findItem(*info);
	if (!item)
	{
		WARN_LOG("iteminfo id is not exist in inven [infoid:%u]", infoId);
		return;
	}

	auto& equip = player->getEquipComponent();

	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);

	auto& comp = player->getEquipComponent();
	auto resultCode = comp.insertEquipment(*cacheTx, item->getId());
	if (resultCode != EResultCode::Success)
		return;

	cacheTx->addBusyUpdater(player->createBusyUpdater());
	cacheTx->run();

}

