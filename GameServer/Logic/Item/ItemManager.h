////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		아이템 관리자
///
///	@ date		2024-4-26
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Protocol/Struct/PktTypes.h>

#include "Actor/ActorTypes.h"
#include "Bag.h"
#include "DB/CacheTx.h"
#include "OrmSub/Item.h"

class PktItemChangeData;
class Item;
class ItemManager
{
public:
	Player& _player;
	std::map<PktObjId, ItemPtr> _items;
	std::array<BagPtr, (int32_t)EInventoryType::Max> _bags;

public:
	/// 생성자
	ItemManager(Player& player);

	/// 소멸자
	virtual ~ItemManager();

	/// 아이템을 추가한다.
	bool insertItem(ItemPtr Item);

	/// 아이템을 찾는다.
	const ItemPtr findItem(PktObjId id) const;
	ItemPtr findItem(PktObjId id);
	const ItemPtr findItem(const ItemInfo& info) const;
	ItemPtr findItem(const ItemInfo& info);
	bool findItems(const ItemInfo& info, std::vector<ItemPtr>& outItems) const;

	/// 아이템을 삭제한다.
	void deleteItem(PktObjId id);

	/// 아이템을 DB에 추가한다.
	bool insertItem(
		CacheTx& cacheTx, 
		const ItemInfo&, 
		int32_t count,
		PktItemChangeData* pktItemChangeData = nullptr) const;

	/// 아이템을 DB에서 삭제한다.
	bool deleteItem(
		CacheTx& cacheTx, 
		const ItemInfo&,
		int32_t count, 
		PktItemChangeData* pktItemChangeData = nullptr) const;

	/// 아이템을 DB에서 삭제한다.
	bool deleteItem(
		CacheTx& cacheTx, 
		PktObjId itemId, 
		int32_t count,
		PktItemChangeData* pktItemChangeData = nullptr) const;

	/// 아이템을 DB에서 삭제한다.
	bool deleteItem(
		CacheTx& cacheTx, 
		ItemPtr item, 
		int32_t count,
		PktItemChangeData* pktItemChangeData = nullptr) const;

	/// 내보낸다.
	void exportTo(EInventoryType invenType, std::vector<PktItem>& pktItems) const;

	/// 모든아이템을 내보낸다.
	void exportTo(std::vector<PktItem>& pktItems) const;
protected:
	/// DB객체로 초기화 한다
	bool _initByList(const std::list<std::shared_ptr<DBOrm::Item>>& ormItems);
};
