////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		OrmEquipment 상속 클래스
///
///	@ date		2024-7-18
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms/OrmEquipment.h>

class ItemInfo;
class PktEquipment;
class PktAppearance;
class Equipment : public DBOrm::Equipment
{
public:
	/// 커스텀 캐시에 반영한다.
	virtual void applyCache(DBOrm::OrmObjectPtr origin) const override;

	std::shared_ptr<Equipment> clone() const;
};

