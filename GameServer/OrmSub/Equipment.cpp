

#include "Pch.h"
#include "Equipment.h"

#include <Data/Info/ItemInfo.h>
#include <Protocol/Struct/PktItem.h>

#include "Actor/Player.h"
#include "Logic/Item/InventoryComponent.h"
#include "Logic/PlayerManager.h"
#include "Logic/Item/EquipComponent.h"

void Equipment::applyCache(DBOrm::OrmObjectPtr origin) const
{
	switch (getCrudState())
	{
	case DBOrm::OrmObject::CRUD::Insert:
		{
			if (auto equipment = std::dynamic_pointer_cast<Equipment>(origin))
			{
				PlayerPtr player;
				if (PlayerManager::Instance().get(_ownerId, player))
				{
					player->getEquipComponent().insertEquipment(equipment);
				}
			}
		}
		break;

	case DBOrm::OrmObject::CRUD::Update:
		{
		}
		break;

	default:
		break;
	}
}

std::shared_ptr<Equipment> Equipment::clone() const
{
	auto clone = std::make_shared<Equipment>();
	copyMember(*clone);

	return clone;
}
