////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktLoginReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktLoginReqHandler.h"

#include <Core/Util/IdFactory.h>
#include <DB/DBTask.h>
#include <DB/Orms/OrmAccounts.h>
#include <DB/Orms/OrmUser.h>
#include <Protocol/Struct/PktSystem.h>

#include "../User.h"
#include "../UserManager.h"
#include "GameServer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktLoginReqHandler::onHandler(User& user, PktLoginReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktLoginReqHandler::_onHandler(User& user, PktLoginReq& req, std::shared_ptr<Ack>& ack)
{
	auto latency = (int32_t)(Core::Time::GetCurTimeMSec() - user.getLatencyCheckTime()) / 2;
	user.setLatency(latency);

	if (req.getUserId() == 0)
	{
		WARN_LOG("userId not set");
		ack->setResult(EResultCode::ParamError);
		user.send(*ack);
		return;
	}

	if (user.getId() != 0 || user.getPlayer())
	{
		WARN_LOG("already user or player exist [uid:%llu, req uid:%llu", user.getId(), req.getUserId());
		ack->setResult(EResultCode::ParamError);
		user.send(*ack);
		return;
	}

	INFO_LOG("user login. [userId:%llu, latency:%d]", req.getUserId(), latency);

	UserPtr prevUser;
	if (UserManager::Instance().get(req.getUserId(), prevUser))
	{
		prevUser->forceClose();
	}

	auto userPtr = user.getUserPtr();
	if (!UserManager::Instance().insert(req.getUserId(), userPtr))
	{
		WARN_LOG("user aleardy exist. [userId:%llu]", req.getUserId());
		ack->setResult(EResultCode::ParamError);
		user.send(*ack);
		return;
	}

	user.setId(req.getUserId());
	user.setAccount(req.getAccount());

	DBTask::Instance().runTask(Caller, [userPtr, ack](const DataBaseTarget& db) mutable
		{
			auto ormUser = DBOrm::User::Select(db, userPtr->getId());
			if (!ormUser)
			{
				ormUser = std::make_shared<DBOrm::User>();
				ormUser->setId(userPtr->getId());
				ormUser->setCreatedTime(Core::Time::GetCurTime());
				if (!ormUser->insert(db))
				{
					ack->setResult(EResultCode::DBError);
					userPtr->send(*ack);
					return false;
				}
			}

			ack->setResult(EResultCode::Success);
			ack->setUserId(userPtr->getId());
			ack->setAccount(userPtr->getAccount());
			ack->setLastPlayedPlayerId(ormUser->getLastPlayerId());

			userPtr->send(*ack);

			return true;
		});

	/// 마지막 접속서버를 로비DB에 업데이트한다.
	auto userId = user.getId();
	DBTask::Instance().runTask(Caller, [userId](const DataBaseTarget& db) mutable
		{
			DBOrm::Accounts account;
			account.setUid(userId);
			account.setLastServerId(GameServer::Instance().GetServerId());
			account.update(db);
			return true;
		}, nullptr, EProjDBType::Login);
}


