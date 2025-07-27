

#include "Pch.h"
#include "Bag.h"

#include <Core/Util/IdFactory.h>
#include <Data/Info/ItemInfo.h>
#include <Protocol/Struct/PktItem.h>

#include "Actor/ActorFactory.h"
#include "Actor/Player.h"
#include "OrmSub/Item.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
Bag::Bag(Player& player, EInventoryType type) : _player(player), _inventoryType(type)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
Bag::~Bag()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  아이템을 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Bag::insertItem(ItemPtr item)
{
	ENSURE(item, return false);

	auto ret = _items.insert(std::make_pair(item->getId(), item));
	if (!ret.second)
	{
		WARN_LOG("duplcated");
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  아이템을 찾는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
const ItemPtr Bag::findItem(PktObjId id) const
{
	auto iter = _items.find(id);
	if (iter == _items.end())
		return nullptr;

	return iter->second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  아이템을 찾는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
ItemPtr Bag::findItem(PktObjId id)
{
	auto iter = _items.find(id);
	if (iter == _items.end())
		return nullptr;

	return iter->second;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템을 찾는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
const ItemPtr Bag::findItem(const ItemInfo& info) const
{
	for (const auto& item : _items | std::views::values)
	{
		if (item->getInfoId() == info.getId())
			return item;
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템을 찾는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
ItemPtr Bag::findItem(const ItemInfo& info)
{
	return std::const_pointer_cast<Item>(static_cast<const Bag&>(*this).findItem(info));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템을 찾는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Bag::findItems(const ItemInfo& info, std::vector<ItemPtr>& outItems) const
{
	for (const auto& item : _items | std::views::values)
	{
		if (item->getInfoId() == info.getId())
			outItems.push_back(item);
	}

	return !outItems.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템을 삭제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Bag::deleteItem(PktObjId id)
{
	_items.erase(id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템을 패킷으로 내보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Bag::exportTo(std::vector<PktItem>& pktItems) const
{
	for (auto itemPair : _items)
	{
		auto item = itemPair.second;

		PktItem pktItem;
		item->exportTo(pktItem);
		pktItems.emplace_back(std::move(pktItem));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  아이템을 DB에 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Bag::insertItem(
	CacheTx& cacheTx, 
	const ItemInfo& itemInfo,
	int32_t count,
	PktItemChangeData* pktItemChangeData) const
{
	int32_t remainCount = count;
	for (auto itemPair : _items)
	{
		auto item = itemPair.second;
		const auto& info = item->getInfo();

		if (info.getId() != itemInfo.getId())
			continue;

		auto accumMax = std::max(info.getAccumMax(), 1);
		if (accumMax <= item->getAccum())
			continue;

		auto cacheItem = cacheTx.acquireObject(_player, item);
		auto enableCount = accumMax - cacheItem->getAccum();
		if (enableCount < remainCount)
		{
			remainCount -= enableCount;
			cacheItem->setAccum(cacheItem->getAccum() + enableCount);
		}
		else
		{
			cacheItem->setAccum(cacheItem->getAccum() + remainCount);
			remainCount = 0;
		}
		cacheItem->updateCache();

		if (pktItemChangeData)
		{
			PktItemCountChange pktItemCountChange(
				item->getId(), cacheItem->getAccum() - item->getAccum(), cacheItem->getAccum());
			pktItemChangeData->getChangedCountItems().emplace_back(std::move(pktItemCountChange));
		}
	}

	while (remainCount > 0)
	{
		auto accumMax = std::max(itemInfo.getAccumMax(), 1);
		auto accum = remainCount > accumMax ? accumMax : remainCount;
		remainCount -= accum;

		auto newItem = std::make_shared<Item>(itemInfo);
		newItem->setId(Core::IdFactory::CreateId());
		newItem->setOwnerId(_player.getId());
		newItem->setInfoId(itemInfo.getId());
		newItem->setAccum(accum);
		auto cacheItem = cacheTx.acquireObject(_player, newItem);
		cacheItem->insertCache();

		if (pktItemChangeData)
		{
			if (accumMax > 1)
			{
				PktItemSimple pktItemSimple;
				newItem->exportTo(pktItemSimple);
				pktItemChangeData->getAcquireAccumItems().emplace_back(std::move(pktItemSimple));
			}
			else
			{
				PktItem pktItem;
				newItem->exportTo(pktItem);
				pktItemChangeData->getAcquireItems().emplace_back(std::move(pktItem));
			}
		}
	}

	_player.broadcastAcquireItem(cacheTx, itemInfo);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템을 DB에서 삭제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Bag::deleteItem(
	CacheTx& cacheTx, 
	const ItemInfo& itemInfo,
	int32_t count,
	PktItemChangeData* pktItemChangeData) const
{
	int32_t remainCount = count;
	for (auto itemPair : _items)
	{
		auto item = itemPair.second;
		const auto& info = item->getInfo();

		if (info.getId() != itemInfo.getId())
			continue;

		if (remainCount == 0)
			break;

		auto cacheItem = cacheTx.acquireObject(_player, item);
		if (item->getAccum() <= remainCount)
		{
			remainCount -= item->getAccum();
			cacheItem->setOwnerId(0);
			cacheItem->setAccum(0);
		}
		else
		{
			cacheItem->setAccum(cacheItem->getAccum() - remainCount);
			remainCount = 0;
		}
		cacheItem->updateCache();

		if (pktItemChangeData)
		{
			PktItemCountChange pktItemCountChange(
				item->getId(), cacheItem->getAccum() - item->getAccum(), cacheItem->getAccum());
			pktItemChangeData->getChangedCountItems().emplace_back(std::move(pktItemCountChange));
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템을 DB에서 삭제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Bag::deleteItem(
	CacheTx& cacheTx, 
	PktObjId itemId, 
	int32_t count,
	PktItemChangeData* pktItemChangeData) const
{
	auto item = findItem(itemId);
	if (!item)
		return false;

	return deleteItem(cacheTx, item, count, pktItemChangeData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템을 DB에서 삭제한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Bag::deleteItem(
	CacheTx& cacheTx, 
	ItemPtr item, 
	int32_t count,
	PktItemChangeData* pktItemChangeData) const
{
	ENSURE(item, return false);

	if (item->getAccum() < count)
		return false;

	auto cacheItem = cacheTx.acquireObject(_player, item);
	cacheItem->setAccum(cacheItem->getAccum() - count);
	if (cacheItem->getAccum() == 0)
		cacheItem->setOwnerId(0);
	cacheItem->updateCache();

	if (pktItemChangeData)
	{
		PktItemCountChange pktItemCountChange(
			item->getId(), cacheItem->getAccum() - item->getAccum(), cacheItem->getAccum());
		pktItemChangeData->getChangedCountItems().emplace_back(std::move(pktItemCountChange));
	}

	return true;
}




