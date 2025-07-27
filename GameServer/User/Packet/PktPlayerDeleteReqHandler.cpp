////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPlayerDeleteReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktPlayerDeleteReqHandler.h"

#include <DB/Orms/OrmServerUserSimpleInfo.h>
#include <Protocol/Struct/PktPlayer.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPlayerDeleteReqHandler::onHandler(User& user, PktPlayerDeleteReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	ack->setPlayerId(req.getPlayerId());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPlayerDeleteReqHandler::_onHandler(User& user, PktPlayerDeleteReq& req, std::shared_ptr<Ack>& ack)
{
	if (user.getId() == 0)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	auto userPtr = user.getUserPtr();
	auto userId = user.getId();

	std::shared_ptr<EResultCode> resultCode = std::make_shared<EResultCode>();

	auto transactionMSec = Core::Time::GetCurTimeMSec();
	auto resultCallback = [ack, userPtr, resultCode, transactionMSec](bool result)
		{
			if (*resultCode != EResultCode::Success)
			{
				ack->setResult(*resultCode);
				userPtr->send(*ack);
				return;
			}

			userPtr->send(*ack);
		};

	DBTask::Instance().runTask(Caller, [userId, resultCode, req](const DataBaseTarget& db) mutable
		{
			auto curTime = Core::Time::GetCurTime();

			auto deletePlayer = DBOrm::Player::Select(db, req.getPlayerId());
			if (!deletePlayer)
			{
				WARN_LOG("player not exist. delete failed. [uid:%llu, pid:%llu]", userId, req.getPlayerId());
				*resultCode = EResultCode::InvalidActorId;
				return false;
			}

			if (!deletePlayer->deleteKey(db))
			{
				WARN_LOG("player not exist. delete failed. [uid:%llu]", userId);
				*resultCode = EResultCode::DBError;
				return false;
			}

			INFO_LOG("player delete. success [uid:%llu, pid:%llu, nick:%s]",
				userId, deletePlayer->getId(), deletePlayer->getNick().c_str());

			auto ormPlayers = DBOrm::Player::SelectListByUserId(db, userId);
			auto characterCount = ormPlayers.size();

			/// 케릭터 수를 로비DB에 업데이트한다.
			DBTask::Instance().runTask(Caller, [userId, characterCount](const DataBaseTarget& db)
				{
					auto ormServerUser = DBOrm::ServerUserSimpleInfo::Select(db, userId, GameServer::Instance().GetServerId());
					if (!ormServerUser)
					{
						ormServerUser = std::make_shared<DBOrm::ServerUserSimpleInfo>();
						ormServerUser->setCharacterCount((uint16_t)characterCount);
						ormServerUser->setId(userId);
						ormServerUser->setServerId(GameServer::Instance().GetServerId());
						ormServerUser->insert(db);
					}
					else
					{
						ormServerUser->setCharacterCount((uint16_t)characterCount);
						ormServerUser->update(db);
					}
					return true;
				}, nullptr, EProjDBType::Login);

			return true;
		}, resultCallback);
}


