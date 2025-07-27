////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		OrmCharacterSlot 상속 클래스
///
///	@ date		2024-7-18
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms/OrmCharacterDeck.h>

class CharacterInfo;
class CharacterDeck : public DBOrm::CharacterDeck
{
public:
	/// 커스텀 캐시에 반영한다.
	virtual void applyCache(DBOrm::OrmObjectPtr origin) const override;

	/// 복제한다.
	std::shared_ptr<CharacterDeck> clone() const;

	/// 슬롯을 설정/반환한다.
	void setCharacterSlot(ECharacterSlot slotType, uint64_t characterId);
	uint64_t getCharacterSlot(ECharacterSlot slotType) const;

	/// 내보낸다.
	void exportTo(std::map<ECharacterSlot, uint64_t>& dst) const;

	/// 스킬그룹이 존재하는지 여부를 반환한다
	ECharacterSlot findSlot(uint64_t characterId) const;
};
