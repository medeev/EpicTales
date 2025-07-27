

#include "Pch.h"
#include "PlayerCharacter.h"

#include <Protocol/Struct/PktPlayer.h>
#include <Protocol/Struct/PktGrowth.h>

#include "Logic/Character/CharacterComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  캐쉬에 반영한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerCharacter::applyCache(DBOrm::OrmObjectPtr origin) const
{
	switch (getCrudState())
	{
	case DBOrm::OrmObject::CRUD::Insert:
		{
			if (auto character = std::dynamic_pointer_cast<PlayerCharacter>(origin))
			{
				PlayerPtr player;
				if (PlayerManager::Instance().get(_ownerId, player))
					player->getCharacterComponent().insertCharacter(character);
			}
		}
		break;

	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  클론한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<PlayerCharacter> PlayerCharacter::clone() const
{
	auto clone = std::make_shared<PlayerCharacter>(_info);
	copyMember(*clone);
	return clone;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  내보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerCharacter::exportTo(PktPlayerCharacter& dest) const
{
	dest.setCharacterId(getId());
	dest.setInfoId(getInfoId());
	
	exportTo(dest.getDistrubute());
}

void PlayerCharacter::exportTo(PktStatPointDistribute& dest) const
{
	dest.setRemain(getRemain());
	dest.setStr(getStr());
	dest.setIntellect(getIntellect());
	dest.setDex(getDex());
}
