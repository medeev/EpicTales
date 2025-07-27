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
#include "OrmSub/Item.h"

class Item;
class Player;
class InventoryComponent
	:
	public IComponent, 
	public ItemManager
{
protected:
	Player& _player; ///< 플레이어

public:
	/// 생성자
	InventoryComponent(Player& actor);

	/// 소멸자
	virtual ~InventoryComponent();

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// DB객체로 초기화 한다
	virtual bool initializeDB(const DataBaseTarget&) override;

	/// 정리 한다
	virtual void finalize() override;

};
