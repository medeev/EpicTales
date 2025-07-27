////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		OrmPlayerCharacter 상속 클래스
///
///	@ date		2024-3-7
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms/OrmPlayerCharacter.h>

class PktStatPointDistribute;
class PktPlayerCharacter;
class PlayerCharacter : public DBOrm::PlayerCharacter
{
protected:
	const CharacterInfo& _info;
public:
	PlayerCharacter(const CharacterInfo& characterInfo) : _info(characterInfo) {}

	/// 커스텀 캐시에 반영한다.
	virtual void applyCache(DBOrm::OrmObjectPtr origin) const override;

	/// 클론한다.
	std::shared_ptr<PlayerCharacter> clone() const;

	/// 내보낸다.
	void exportTo(PktPlayerCharacter& dest) const;
	/// 분배포인트만 내보낸다
	void exportTo(PktStatPointDistribute& dest) const;
};

typedef std::shared_ptr<PlayerCharacter> PlayerCharacterPtr;