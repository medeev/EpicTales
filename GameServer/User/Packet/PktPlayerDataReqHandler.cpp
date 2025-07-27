////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPlayerDataReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktPlayerDataReqHandler.h"

#include <Core/Util/IdFactory.h>
#include <Data/Info/ConstInfo.h>
#include <DB/DBTask.h>
#include <DB/Orms/OrmPlayer.h>
#include <DB/Orms/OrmUser.h>
#include <Protocol/Struct/PktPlayer.h>
#include <Protocol/Struct/PktSystem.h>

#include "../User.h"
#include "Actor/Player.h"
#include "Logic/PlayerManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPlayerDataReqHandler::onHandler(User& user, PktPlayerDataReq& req)
{
    std::shared_ptr<Ack> ack = std::make_shared<Ack>();
    ack->setReqKey(req.getReqKey());
    _onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPlayerDataReqHandler::_onHandler(User& user, PktPlayerDataReq& req, std::shared_ptr<Ack>& ack)
{
	if (user.getId() == 0)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	auto playerId = req.getId();
	if (playerId == 0)
	{
		ack->setResult(EResultCode::ParamError);
		user.send(*ack);
		return;
	}

	auto userPtr = user.getUserPtr();
	
	PlayerWeakPtr prevPlayerWeak;
	PlayerPtr prevPlayPlayer;
	if (PlayerCloseManager::Instance().get(user.getId(), prevPlayerWeak))
	{
		prevPlayPlayer = prevPlayerWeak.lock();
		if (!prevPlayPlayer)
			PlayerCloseManager::Instance().remove(user.getId());
	}

	RoomPtr room;
	PlayerPtr player;
	if (PlayerManager::Instance().get(playerId, player))
	{
		INFO_LOG("player already exist. player and user change [pid:%llu]", playerId);

		userPtr->possess(player);

		room = player->getRoomPtr();
	}

	if (prevPlayPlayer && prevPlayPlayer != player)
	{
		prevPlayPlayer->setUser(nullptr);

		if (auto prevPlayerRoom = prevPlayPlayer->getRoom())
		{
			prevPlayerRoom->runTask(Caller, [prevPlayPlayer](RoomThreadContextRef context)
				{
					prevPlayPlayer->reconnectOldPlayerProcess(context);
				});
		}
	}

	if (player && room)
	{
		player->runTask(Caller, [player, ack]()
			{
				player->exportTo(ack->getPlayerData());
				player->send(*ack);
			});
	}
	else
	{
		DBTask::Instance().runTask(Caller, [ack, userPtr, player, playerId](const DataBaseTarget& db) mutable
			{
				auto curTime = Core::Time::GetCurTime();
				auto curstr = curTime.toString();

				DBOrm::User ormUser;
				ormUser.setId(userPtr->getId());
				ormUser.setLastPlayerId(playerId);
				if (!ormUser.update(db))
					WARN_LOG("lastPlayerId update failed");

				if (!player)
				{
					player = std::make_shared<Player>();
					player->setId(playerId);
					player->setPtr(player);
					player->initialize();

					if (!player->initializeDB(db))
					{
						userPtr->unPossess();

						ack->setResult(EResultCode::DBError);
						userPtr->send(*ack);
						return false;
					}
					else
					{
						PlayerManager::Instance().insert(player->getId(), player);
					}
				}

				userPtr->possess(player);

				auto task = [player, ack]()
					{
						player->exportTo(ack->getPlayerData());
						player->send(*ack);
					};

				if (auto room = player->getRoom())
				{
					player->runTask(Caller, [task]()
						{
							task();
						});
				}
				else
				{
					task();
				}


				return true;
			});
	}


	
}


