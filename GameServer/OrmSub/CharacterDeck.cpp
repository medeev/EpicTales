

#include "Pch.h"
#include "CharacterDeck.h"

#include <Data/Info/CharacterInfo.h>

#include "Actor/Player.h"
#include "Logic/PlayerManager.h"
#include "Logic/Character/CharacterComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  커스텀캐시에 반영한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterDeck::applyCache(DBOrm::OrmObjectPtr origin) const
{
	switch (getCrudState())
	{
	case DBOrm::OrmObject::CRUD::Insert:
		{
			PlayerPtr player;
			if (PlayerManager::Instance().get(_ownerId, player))
				player->getCharacterComponent().insertCharacterSlot(this->clone());
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  내보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterDeck::exportTo(std::map<ECharacterSlot, uint64_t>& dst) const
{
	dst[ECharacterSlot::Slot0] = getCharacter0();
	dst[ECharacterSlot::Slot1] = getCharacter1();
	dst[ECharacterSlot::Slot2] = getCharacter2();
	dst[ECharacterSlot::Slot3] = getCharacter3();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  슬롯을 찾는다.
////////////////////////////////////////////////////////////////////////////////////////////////////
ECharacterSlot CharacterDeck::findSlot(uint64_t characterId) const
{
	if (getCharacter0() == characterId)
		return ECharacterSlot::Slot0;
	else if (getCharacter1() == characterId)
		return ECharacterSlot::Slot1;
	else if (getCharacter2() == characterId)
		return ECharacterSlot::Slot2;
	else if (getCharacter3() == characterId)
		return ECharacterSlot::Slot3;

	return ECharacterSlot::Max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  복사본을 생성한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<CharacterDeck> CharacterDeck::clone() const
{
	auto clone = std::make_shared<CharacterDeck>();
	copyMember(*clone);

	return clone;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  슬롯을 설정한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterDeck::setCharacterSlot(ECharacterSlot slotType, uint64_t characterId)
{
	switch (slotType)
	{
	case ECharacterSlot::Slot0:
		setCharacter0(characterId);
		break;
	case ECharacterSlot::Slot1:
		setCharacter1(characterId);
		break;
	case ECharacterSlot::Slot2:
		setCharacter2(characterId);
		break;
	case ECharacterSlot::Slot3:
		setCharacter3(characterId);
		break;
	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  슬롯의 스킬그룹정보를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t CharacterDeck::getCharacterSlot(ECharacterSlot slotType) const
{
	uint64_t characterId = 0;
	switch (slotType)
	{
	case ECharacterSlot::Slot0:
		characterId = getCharacter0();
		break;
	case ECharacterSlot::Slot1:
		characterId = getCharacter1();
		break;
	case ECharacterSlot::Slot2:
		characterId = getCharacter2();
		break;
	case ECharacterSlot::Slot3:
		characterId = getCharacter3();
		break;
	default:
		WARN_LOG("invalid slot type");
		break;
	}

	return characterId;
}
