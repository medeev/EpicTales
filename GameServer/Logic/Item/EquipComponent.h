////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		컴포넌트 클래스
///
///	@ date		2024-4-26
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Protocol/Struct/PktTypes.h>

#include "Actor/ActorTypes.h"
#include "Component/IComponent.h"
#include "ItemManager.h"
#include "OrmSub/Equipment.h"
#include "OrmSub/Item.h"

class CacheTx;
class Item;
class Player;
class EquipComponent
	:
	public IComponent
{
protected:
	Player& _player; ///< 플레이어
	std::map<uint64_t, EquipmentPtr> _equips;

public:
	/// 생성자
	EquipComponent(Player& actor);

	/// 소멸자
	virtual ~EquipComponent();

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// DB객체로 초기화 한다
	virtual bool initializeDB(const DataBaseTarget&) override;

	/// 정리 한다
	virtual void finalize() override;

public:
	/// 추가한다.
	bool insertEquipment(EquipmentPtr equip);

	/// 변경한다.
	EResultCode insertEquipment(CacheTx& cacheTx, uint64_t itemId) const;

private:
	void _refreshEquipItemStat(PktActorStats* changedList = nullptr) const;

};
