

#include "Pch.h"
#include "EquipComponent.h"

#include <Data/Info/ItemInfo.h>
#include <DB/Orms/OrmEquipment.h>
#include <Protocol/Struct/PktItem.h>

#include "Actor/ActorFactory.h"
#include "Actor/Player.h"
#include "OrmSub/Equipment.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
EquipComponent::EquipComponent(Player& player)
	:
	IComponent(EComponentType::Equip, player),
	_player(player)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
EquipComponent::~EquipComponent()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void EquipComponent::initialize()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		정리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void EquipComponent::finalize()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  장착아이템에 대한 스탯을 갱신한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void EquipComponent::_refreshEquipItemStat(PktActorStats* changedList) const
{
	ActorStat allItemStat;
	for (const auto& equip : _equips | std::views::values)
	{
		if (!equip->getItemId())
			continue;

		ItemInfoPtr itemInfo(equip->getItemInfoId());
		if (!itemInfo)
		{
			WARN_LOG("item info not exist [ownerId:%llu, itemInfo:%u]", _player.getId(), equip->getItemInfoId());
			continue;
		}

		ActorStat oneItemStat(*itemInfo);
		allItemStat += oneItemStat;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	DB객체로 초기화한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool EquipComponent::initializeDB(const DataBaseTarget& conn)
{
	auto ormEquips = DBOrm::Equipment::SelectListByOwnerId(
		conn, _player.getId());

	for (auto equipOrm : ormEquips)
	{
		auto infoId = equipOrm->getItemInfoId();

		if (infoId != 0)
		{
			ItemInfoPtr itemInfo(infoId);
			if (!itemInfo)
			{
				WARN_LOG("item info not exist. [ownerId:%llu, itemInfoId: %u",
					equipOrm->getOwnerId(), infoId);

				continue;
			}
		}

		auto quipment = std::make_shared<Equipment>();
		equipOrm->copyMember(*quipment);

		insertEquipment(quipment);
	}

	if (!_equips.empty())
		_refreshEquipItemStat();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  장착정보를 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool EquipComponent::insertEquipment(EquipmentPtr equip)
{
	ENSURE(equip, return false);

	auto ret = _equips.insert(std::make_pair(equip->getItemId(), equip));
	if (!ret.second)
	{
		WARN_LOG("equip is duplcated. [ownerId:%llu, itemId:%llu]", equip->getOwnerId(), equip->getItemId());
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  장착아이템을 변경한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
EResultCode EquipComponent::insertEquipment(
	CacheTx& cacheTx, uint64_t itemId) const
{
	auto iter = _equips.find(itemId);
	if (iter != _equips.end())
		return EResultCode::AlreadyExistItem;


	ItemPtr item;
	if (itemId)
	{
		item = _player.getInvenComponent().findItem(itemId);
		if (!item)
			return EResultCode::NotExistItemById;

		auto curEquip = std::make_shared<Equipment>();
		curEquip->setOwnerId(_player.getId());
		curEquip->setItemId(itemId);
		curEquip->setItemInfoId(item->getInfoId());
		auto cacheEquip = cacheTx.acquireObject(_player, curEquip);
		cacheEquip->insertCache();
	}
	

	if (item)
	{
		_player.broadcastEquipItem(cacheTx, *item);
	}

	auto player = _player.shared_from_this<Player>();
	cacheTx.ifSucceed(_player, [player, this, item]()
		{
			auto itemId = item ? item->getId() : 0;
			{
				
			}
			
			{
				PktActorStatChangeNotify notify;
				_refreshEquipItemStat(&notify.getStatList());
				player->send(notify);
			}
		});

	return EResultCode::Success;
}
