////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		인벤토리 백
///
///	@ date		2024-4-29
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Data/Info/InfoEnums.h>
#include <Protocol/Struct/PktTypes.h>

#include "DB/CacheTx.h"
#include "OrmSub/OrmSubType.h"

class PktItemChangeData;
class Player;
class Item;
class ItemInfo;
class Bag
{
	Bag(const Bag&) = delete;
	Bag& operator=(const Bag&) = delete;
	Bag() = delete;

protected:
	Player& _player;
	std::map<PktObjId, ItemPtr> _items;
	EInventoryType _inventoryType;
public:
	/// 생성자
	Bag(Player& player, EInventoryType type);

	/// 소멸자
	virtual ~Bag();

	bool insertItem(ItemPtr Item);

	const ItemPtr findItem(PktObjId id) const;
	ItemPtr findItem(PktObjId id);
	const ItemPtr findItem(const ItemInfo& info) const;
	ItemPtr findItem(const ItemInfo& info);
	bool findItems(const ItemInfo& info, std::vector<ItemPtr>& outItems) const;

	void deleteItem(PktObjId id);
	void exportTo(std::vector<PktItem>& pktItems) const;

	bool insertItem(
		CacheTx& cacheTx, 
		const ItemInfo& info,
		int32_t count,
		PktItemChangeData* pktItemChangeData
	) const;
	bool deleteItem(
		CacheTx& cacheTx, 
		const ItemInfo& info,
		int32_t count,
		PktItemChangeData* pktItemChangeData
	) const;
	bool deleteItem(
		CacheTx& cacheTx, 
		PktObjId itemId, 
		int32_t count,
		PktItemChangeData* pktItemChangeData) const;
	bool deleteItem(
		CacheTx& cacheTx, 
		ItemPtr item, 
		int32_t count,
		PktItemChangeData* pktItemChangeData) const;

protected:
};

typedef std::shared_ptr<Bag> BagPtr;
