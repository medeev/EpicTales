

#include "Pch.h"
#include "Skill.h"

#include <Data/Info/SkillInfo.h>

#include "Actor/Player.h"
#include "Logic/PlayerManager.h"
#include "Logic/Skill/SkillComponent.h"

void Skill::applyCache(DBOrm::OrmObjectPtr origin) const
{
	switch (getCrudState())
	{
	case DBOrm::OrmObject::CRUD::Insert:
		{
			/// 플레이어 인벤토리면 여기서 처리한다.
			if (auto skill = std::dynamic_pointer_cast<Skill>(origin))
			{
				PlayerPtr player;
				if (PlayerManager::Instance().get(_ownerId, player))
				{
					player->getSkillComponent().insertSkill(skill);
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


std::shared_ptr<Skill> Skill::clone() const
{
	auto clone = std::make_shared<Skill>(_skillInfo);
	copyMember(*clone);

	return clone;
}
