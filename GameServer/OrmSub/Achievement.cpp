

#include "Pch.h"
#include "Achievement.h"

#include <Data/Info/AchievementInfo.h>
#include <Protocol/Struct/PktAchievement.h>

#include "Actor/Player.h"
#include "Logic/Achievement/AchievementComponent.h"
#include "Logic/PlayerManager.h"

void Achievement::applyCache(DBOrm::OrmObjectPtr origin) const
{
	switch (getCrudState())
	{
	case DBOrm::OrmObject::CRUD::Insert:
		{
			if (auto achievement = std::dynamic_pointer_cast<Achievement>(origin))
			{
				PlayerPtr player;
				if (PlayerManager::Instance().get(_ownerId, player))
				{
					player->getAchievementComponent().insertAchievement(achievement);
				}
			}
			/// 공용인벤토리면 여기서 처리한다.


			/// 기타 다른 인벤토리면 여기서 처리한다.
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

void Achievement::exportTo(PktAchievement& dest) const
{
	dest.setAchievementInfoid(getInfoId());
	dest.setStatus((EAchivementState)getStatus());
}

std::shared_ptr<Achievement> Achievement::clone() const
{
	auto clone = std::make_shared<Achievement>(_achievementInfo);
	copyMember(*clone);

	return clone;
}
