

#include "Pch.h"
#include "Mail.h"

#include <Protocol/Struct/PktMail.h>

#include "Actor/Player.h"
#include "Logic/Mail/MailComponent.h"
#include "Logic/PlayerManager.h"

void Mail::applyCache(DBOrm::OrmObjectPtr origin) const
{
	switch (getCrudState())
	{
	case DBOrm::OrmObject::CRUD::Insert:
		{
			if (auto mail = std::dynamic_pointer_cast<Mail>(origin))
			{
				PlayerPtr player;
				if (PlayerManager::Instance().get(_ownerId, player))
					player->getMailComponent().insertMail(mail);
			}
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  클론한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<Mail> Mail::clone() const
{
	auto clone = std::make_shared<Mail>();
	copyMember(*clone);

	return clone;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  퀘스트 정보를 내보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Mail::exportTo(PktMail& pktMail) const
{
	pktMail.setId(getId());
	pktMail.setSenderId(getSenderId());
	pktMail.setExpireTime(getExpireTime().getTimeValue());
	pktMail.setCreatedTime(getCreatedTime().getTimeValue());
	pktMail.setTitle(getTitle());
	pktMail.setContent(getContent());
}
