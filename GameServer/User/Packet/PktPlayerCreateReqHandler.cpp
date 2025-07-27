////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPlayerCreateReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktPlayerCreateReqHandler.h"

#include <Data/Info/CharacterInfo.h>
#include <Data/Info/ConstInfo.h>
#include <Data/Info/ItemInfo.h>
#include <Data/Info/WorldInfo.h>
#include <DB/Orms/OrmServerUserSimpleInfo.h>

#include "Data/WorldEntityData/WorldEntityData.h"
#include "DB/Orms/OrmPlayerCharacter.h"
#include "GameServer.h"
#include "DB/Orms/OrmCharacterDeck.h"
#include "DB/Orms/OrmEquipment.h"
#include "DB/Orms/OrmItem.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPlayerCreateReqHandler::onHandler(User& user, PktPlayerCreateReq& req)
{
    std::shared_ptr<Ack> ack = std::make_shared<Ack>();
    ack->setReqKey(req.getReqKey());
    _onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPlayerCreateReqHandler::_onHandler(User& user, PktPlayerCreateReq& req, std::shared_ptr<Ack>& ack)
{
	if (user.getId() == 0)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	if (req.getNick().empty())
	{
		ack->setResult(EResultCode::ParamError);
		user.send(*ack);
		return;
	}

	auto userPtr = user.getUserPtr();
	auto userId = user.getId();
	auto characterInfoId = req.getRepresentCharInfoId();

	auto insertPlayer = std::make_shared<DBOrm::Player>();
	std::shared_ptr<EResultCode> resultCode = std::make_shared<EResultCode>();

	auto transactionMSec = Core::Time::GetCurTimeMSec();
	auto resultCallback = [ack, userPtr, insertPlayer, resultCode, transactionMSec, characterInfoId](bool result)
		{
			if (*resultCode != EResultCode::Success)
			{
				ack->setResult(*resultCode);
				userPtr->send(*ack);
				return;
			}

			PktPlayerSimple pktPlayerSimple;
			pktPlayerSimple.setId(insertPlayer->getId());
			pktPlayerSimple.setRepresentCharacterId(insertPlayer->getRepresentCharacterId());
			pktPlayerSimple.setRepresentCharacterInfoId(characterInfoId);
			pktPlayerSimple.setNick(insertPlayer->getNick());

			ack->setPlayerSimple(pktPlayerSimple);

			userPtr->send(*ack);

			INFO_LOG("transaction elasped : %lld", Core::Time::GetCurTimeMSec() - transactionMSec);
		};

	DBTask::Instance().runTask(Caller, [userId, resultCode, req, insertPlayer](const DataBaseTarget& db) mutable
		{
			auto curTime = Core::Time::GetCurTime();

			if (auto nickPlayer = DBOrm::Player::SelectByNick(db, req.getNick()))
			{
				*resultCode = EResultCode::AlreadyExistNick;
				return false;
			}

			auto ormPlayers = DBOrm::Player::SelectListByUserId(db, userId);

			/// 케릭터 카운트를 체크한다.
			auto characterCount = ormPlayers.size();
			if (ormPlayers.size())
			{
			}

			WorldInfoPtr worldInfo(ConstInfoManager::Instance().getDefaultStartVilliageInfoId());
			if (!worldInfo)
			{
				WARN_LOG("worldinfo not exist. player create failed. [uid:%llu, worldInfo:%u]"
					, userId, ConstInfoManager::Instance().getDefaultStartVilliageInfoId());
				*resultCode = EResultCode::InfoNotExist;
				return false;
			}

			auto playerStart = worldInfo->getPlayerStart(
				ConstInfoManager::Instance().getDefaultStartVilliagePlayerStart());
			if (!playerStart)
			{
				*resultCode = EResultCode::InfoNotExist;
				return false;
			}

			const auto& startLocation = playerStart->getLocation();

			CharacterInfoPtr characterInfo(req.getRepresentCharInfoId());
			if (!characterInfo)
			{
				WARN_LOG("player create failed. [uid:%llu]", userId);
				*resultCode = EResultCode::InfoNotExist;
				return false;
			}

			auto playerId = Core::IdFactory::CreateId();
			auto characterId = Core::IdFactory::CreateId();

			DBOrm::PlayerCharacter ormPlayerCharacter;
			ormPlayerCharacter.setId(characterId);
			ormPlayerCharacter.setOwnerId(playerId);
			ormPlayerCharacter.setInfoId(req.getRepresentCharInfoId());
			if (!ormPlayerCharacter.insert(db))
			{
				WARN_LOG("player character create failed. [uid:%llu, pid:%llu, cid:%llu]", userId, playerId, characterId);
				*resultCode = EResultCode::DBError;
				return false;
			}

			DBOrm::CharacterDeck ormCharacterDeck;
			ormCharacterDeck.setOwnerId(playerId);
			ormCharacterDeck.setDeckId(0);
			ormCharacterDeck.setCharacter0(characterId);
			ormCharacterDeck.setRepresentCharacterId(characterId);
			if (!ormCharacterDeck.insert(db))
			{
				WARN_LOG("player character deck create failed. [uid:%llu]", userId);
				*resultCode = EResultCode::DBError;
				return false;
			}

			insertPlayer->setUserId(userId);
			insertPlayer->setId(playerId);
			insertPlayer->setRepresentCharacterId(characterId);
			insertPlayer->setNick(req.getNick());
			insertPlayer->setCurWorldInfoId(worldInfo->getId());
			insertPlayer->setCreatedTime(curTime);
			insertPlayer->setLastConnTime(curTime);
			insertPlayer->setX(startLocation.x);
			insertPlayer->setY(startLocation.y);
			insertPlayer->setZ(startLocation.z);

			if (!insertPlayer->insert(db))
			{
				WARN_LOG("player create. insert failed. [uid:%llu]", userId);
				*resultCode = EResultCode::DBError;
				return false;
			}

			/// 시작 아이템을 지급한다.
			if (auto infoId = ConstInfoManager::Instance().getDefaultStartItemInfoId())
			{
				std::map<int32_t, std::pair<const ItemInfo*, uint64_t>> startItemInfos;
				ItemInfoPtr itemInfo(infoId);
				if (itemInfo)
				{
					DBOrm::Item insertItem;
					insertItem.setId(Core::IdFactory::CreateId());
					insertItem.setOwnerId(insertPlayer->getId());
					insertItem.setInfoId(infoId);
					insertItem.setLv(1);
					insertItem.setCreatedTime(curTime);
					insertItem.setAccum(1);
					if (!insertItem.insert(db))
					{
						WARN_LOG("item create. insert failed. [uid:%llu]", userId);
						*resultCode = EResultCode::DBError;
						return false;
					}

					startItemInfos[itemInfo->getId()] = std::make_pair(itemInfo, insertItem.getId());
				}


				if (auto infoId = ConstInfoManager::Instance().getDefaultStartItemEquipItemInfoId())
				{
					auto iter = startItemInfos.find(infoId);
					if (iter != startItemInfos.end())
					{
						auto itemInfoIdPair = iter->second;
						auto itemInfo = itemInfoIdPair.first;
						DBOrm::Equipment insertEquip;
						insertEquip.setOwnerId(insertPlayer->getId());
						insertEquip.setItemId(itemInfoIdPair.second);
						insertEquip.setItemInfoId(itemInfo->getId());

						if (!insertEquip.insert(db))
						{
							WARN_LOG("equip create. insert failed. [uid:%llu]", userId);
							*resultCode = EResultCode::DBError;
							return false;
						}
					}
					else
					{
						WARN_LOG("defaut equip item info id is not exist!! in start items list");
					}
				}

			}

			INFO_LOG("player create. success [uid:%llu, pid:%llu, nick:%s]",
				userId, insertPlayer->getId(), insertPlayer->getNick().c_str());

			/// 케릭터 수를 로비DB에 업데이트한다.
			DBTask::Instance().runTask(Caller, [userId, characterCount](const DataBaseTarget& db)
				{
					auto ormServerUser = DBOrm::ServerUserSimpleInfo::Select(db, userId, GameServer::Instance().GetServerId());
					if (!ormServerUser)
					{
						ormServerUser = std::make_shared<DBOrm::ServerUserSimpleInfo>();
						ormServerUser->setCharacterCount((uint16_t)characterCount + 1);
						ormServerUser->setId(userId);
						ormServerUser->setServerId(GameServer::Instance().GetServerId());
						ormServerUser->insert(db);
					}
					else
					{
						ormServerUser->setCharacterCount((uint16_t)characterCount + 1);
						ormServerUser->update(db);
					}
					return true;
				}, nullptr, EProjDBType::Login);

			return true;
		}, resultCallback);
}


