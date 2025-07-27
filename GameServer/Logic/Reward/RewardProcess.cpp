

#include "Pch.h"
#include "RewardProcess.h"

#include <Data/Info/RewardInfo.h>

#include "Actor/Npc.h"
#include "Actor/Player.h"
#include "Data/Info/ItemInfo.h"
#include "Protocol/Struct/PktItem.h"
#include "Room/World.h"
#include "../Item/ItemManager.h"
#include "../Character/CharacterComponent.h"
#include "../Item/InventoryComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  보상을 실행한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void RewardProcess::Process(
	CacheTx& cacheTx,
	ActorPtr self, PlayerPtr receiver, RewardInfoByGroupIdPtr rewardGroup)
{
	ENSURE(self, return);
	ENSURE(receiver, return);
	ENSURE(rewardGroup, return);
	auto world = receiver->getWorld();
	ENSURE(world, return);


	std::vector<uint32_t> pickupItemInfos;
	for (const auto& reward : *rewardGroup)
	{
		switch (reward->getRewardType())
		{
		case ERewardType::Item:
			{
				ItemInfoPtr itemInfo(reward->getItemInfoId());
				if (!itemInfo)
				{
					WARN_LOG("iteminfo not found [iteminfoId:%u]", reward->getItemInfoId());
					continue;
				}

				auto& inven = receiver->getInvenComponent();

				PktItemChangeNotify notify;

				if (!inven.insertItem(cacheTx, *itemInfo, reward->getItemCount(), &notify.getChangedItemData()))
					return;

				cacheTx.ifSucceed(*receiver, [receiver, notify]()
					{
						receiver->send(notify);
					});
				break;
			}

		default:
			WARN_LOG("reward type process not doing");
			break;

		}
	}

	
	world->spawnPickupItems(*self, *receiver, pickupItemInfos);
}


