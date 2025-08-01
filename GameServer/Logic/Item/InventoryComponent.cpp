

#include "Pch.h"
#include "InventoryComponent.h"

#include <Data/Info/ItemInfo.h>

#include "Actor/ActorFactory.h"
#include "Actor/Player.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
InventoryComponent::InventoryComponent( Player& player )
	:
	IComponent(EComponentType::Inven, player),
	ItemManager(player),
	_player( player )
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
InventoryComponent::~InventoryComponent()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void InventoryComponent::initialize()
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		정리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void InventoryComponent::finalize()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	DB객체로 초기화한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool InventoryComponent::initializeDB(const DataBaseTarget& conn)
{
	auto ormItems = DBOrm::Item::SelectListByOwnerId(
		conn, _player.getId());

	_initByList(ormItems);

	return true;
}
