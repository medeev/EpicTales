////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		OrmItem 상속 클래스
///
///	@ date		2024-4-26
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms/OrmItem.h>

class ItemInfo;
class PktItem;
class PktItemSimple;
class Item : public DBOrm::Item
{
	Item() = delete;
protected:
	const ItemInfo& _itemInfo;
public:
	/// 생성자
	Item(const ItemInfo& itemInfo) : _itemInfo(itemInfo) {}

	/// 커스텀 캐시에 반영한다.
	virtual void applyCache(DBOrm::OrmObjectPtr origin) const override;

	const ItemInfo& getInfo() const {
		return _itemInfo;
	}
	void exportTo(PktItem& dest) const;
	void exportTo(PktItemSimple& dest) const;

	std::shared_ptr<Item> clone() const;
};

