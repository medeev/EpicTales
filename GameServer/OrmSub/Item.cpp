

#include "Pch.h"
#include "Item.h"

#include <Data/Info/ItemInfo.h>
#include <Protocol/Struct/PktItem.h>

#include "Actor/Player.h"
#include "Logic/Item/InventoryComponent.h"
#include "Logic/PlayerManager.h"

void Item::applyCache(DBOrm::OrmObjectPtr origin) const
{
	switch (getCrudState())
	{
	case DBOrm::OrmObject::CRUD::Insert:
		{
			/// 플레이어 인벤토리면 여기서 처리한다.
			if (auto item = std::dynamic_pointer_cast<Item>(origin))
			{
				PlayerPtr player;
				if (PlayerManager::Instance().get(_ownerId, player))
				{
					player->getInvenComponent().insertItem(item);
				}
			}
			/// 공용인벤토리면 여기서 처리한다.


			/// 기타 다른 인벤토리면 여기서 처리한다.
		}
		break;

	case DBOrm::OrmObject::CRUD::Update:
		{
			/// 플레이어 인벤토리면 여기서 처리한다.
			if (getOwnerId() == 0)
			{
				if (auto originItem = std::dynamic_pointer_cast<Item>(origin))
				{
					PlayerPtr player;
					if (PlayerManager::Instance().get(originItem->getOwnerId(), player))
					{
						player->getInvenComponent().deleteItem(getId());
					}
				}
			}
		}
		break;

	default:
		break;
	}
}

void Item::exportTo(PktItem& dest) const
{
	dest.setId(_id);
	dest.setEquipCharacterId(_equipCharacterId);
	dest.setItemInfoid(_infoId);
	dest.setLv(_lv);
	dest.setAccum(_accum);
}

void Item::exportTo(PktItemSimple& dest) const
{
	dest.setId(_id);
	dest.setItemInfoid(_infoId);
	dest.setAccum(_accum);
}

std::shared_ptr<Item> Item::clone() const
{
	auto clone = std::make_shared<Item>(_itemInfo);
	copyMember(*clone);

	return clone;
}
