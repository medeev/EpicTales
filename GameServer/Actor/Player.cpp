

#include "Pch.h"
#include "Player.h"

#include <Core/Thread/IThread.h>
#include <Data/Info/CharacterInfo.h>
#include <DB/Orms/OrmPlayerCharacter.h>
#include <Protocol/Struct/PktItem.h>
#include <Protocol/Struct/PktPlayer.h>
#include <Protocol/Struct/PktRoom.h>

#include "Component/IComponent.h"
#include "Logic/Item/EquipComponent.h"
#include "Logic/Item/InventoryComponent.h"
#include "Logic/Achievement/AchievementComponent.h"
#include "Logic/Character/CharacterComponent.h"
#include "Logic/Character/CloseComponent.h"
#include "Logic/Mail/MailComponent.h"
#include "Logic/Quest/QuestComponent.h"
#include "Logic/Skill/SkillComponent.h"
#include "Logic/PlayerManager.h"
#include "Room/World.h"
#include "User/User.h"
#include "Room/RoomThreadContext.h"
#include "Data/Info/WorldInfo.h"
#include "OrmSub/PlayerCharacter.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
Player::Player()
	:
	super(EActorType::Player),
	_characterComponent(std::make_unique<CharacterComponent>(*this)),
	_inventoryComponent(std::make_unique<InventoryComponent>(*this)),
	_equipComponent(std::make_unique<EquipComponent>(*this)),
	_questComponent(std::make_unique<QuestComponent>(*this)),
	_skillComponent(std::make_unique<SkillComponent>(*this)),
	_mailComponent(std::make_unique<MailComponent>(*this)),
	_achievementComponent(std::make_unique< AchievementComponent>(*this)),
	_closeComponent(std::make_unique<CloseComponent>(*this)),
	_latency(0)
{
	_teamType = ETeamType::Player;
	_lastPublicWorldInfoId = 0;
	_characterInfo = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
Player::~Player()
{
	INFO_LOG("player is deleted. [pid:%llu]", getId());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::initialize()
{
	super::initialize();

	INFO_LOG("player initialize. [pid: %llu, this: %p]", getId(), this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief DB를 이용해 초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Player::initializeDB(const DataBaseTarget& db)
{
	auto orm = DBOrm::Player::Select(db, getId());
	if (!orm)
		return false;

	orm->setLastConnTime(Core::Time::GetCurTime());
	if (!orm->update(db))
	{
		WARN_LOG("player update failed [pid:%llu]", getId());
		return false;
	}


	auto playerDb = std::make_shared<PlayerDB>();
	orm->copyMember(*playerDb);

	_playerDb = playerDb;

	setLocation(_playerDb->getLocation());
	setDirection(_playerDb->getDir());


	for (const auto& component : _components)
	{
		if (!component.second)
			continue;

		if (!component.second->initializeDB(db))
		{
			WARN_LOG("init component is failed [pid:%llu]]", getId());
			return false;
		}
	}

	for (const auto& component : _components)
	{
		if (!component.second)
			continue;

		component.second->initializeDBPost(db);
	}

	_characterInfo = nullptr;
	if (auto character = getCharacterComponent().findPlayerCharacter(orm->getRepresentCharacterId()))
	{
		_characterInfo = CharacterInfoPtr( character->getInfoId());
		ENSURE(_characterInfo, return false);
		setCharacterInfo(_characterInfo);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 정리 한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::finalize()
{
	INFO_LOG("player release. [pid: %llu, this: %p]", getId(), this);

	super::finalize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 유저를 설정한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::setUser(const UserPtr& user)
{
	_userWeakPtr = user;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 유저를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
const UserPtr Player::getUser() const
{
	return _userWeakPtr.lock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 유저를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
UserPtr Player::getUser()
{
	return  _userWeakPtr.lock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 반지름을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
float Player::getRadius() const
{
	return _info ? _info->getCapsuleRadius() : 45.f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 높이값 반을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
float Player::getHalfHeight() const
{
	return _info ? _info->getCapsuleHalfHeight() : 90.f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  레이턴시를 설정한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::setLatency(int32_t latency)
{
	INFO_LOG("latency [pid:%llu latency:%d]", getId(), latency);
	_latency = latency;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 보낸다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Player::send(const PktBase& pktBase) const
{
	auto user = getUser();
	if (!user)
		return false;

	return user->send(pktBase);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 월드에 입장한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::enterWorld(WorldPtr world, const std::function<void(WorldPtr, PlayerPtr)>& callback)
{
	auto self = std::dynamic_pointer_cast<Player>(shared_from_this());

	if (auto room = getWorld())
	{
		WARN_LOG("already exist room.");
		return;
	}

	if (world->getThreadId() != Core::IThread::GetCurThreadId())
	{
		world->increaseEnterReserveCount();
		world->runTask(
			Caller,
			[ this, self, world, callback ](RoomThreadContextRef)
			{
				world->decreaseEnterReserveCount();
				enterWorld( world, callback );
			} );
		return;
	}

	world->addActor(self);

	_fsmComponent.changeState(
		EFsmStateType::PlayerStateIdle);

	if (callback)
		callback(world, self);

	updateSight();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		월드에서 나간다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::leaveWorld(const std::function<void(Player&)>& callback)
{
	auto self = std::dynamic_pointer_cast<Player>(shared_from_this());

	auto room = getRoomPtr();
	if (!room)
	{
		if (callback)
			callback(*this);

		return;
	}
	else if (room->getThreadId() != Core::IThread::GetCurThreadId())
	{
		room->increaseLeaveReserveCount();
		room->runTask(
			Caller,
			[ this, self, room, callback ](RoomThreadContextRef context)
			{
				room->decreaseLeaveReserveCount();
				leaveWorld( callback);
			} );
		return;
	}

	removeSeeingAll();
	setRoomRemovable(true);
	setRoomLeaveCallback(callback);

	updateSight();
	room->setForceUpdateCheckTime(room->getTime() + 10000);
}

void Player::reconnectOldPlayerProcess(RoomThreadContextRef)
{
	PlayerManager::Instance().remove(getId());

	if (auto player = shared_from_this<Player>())
	{
		auto cacheTx = std::make_shared<CacheTx>(Caller);

		auto curTime = Core::Time::GetCurTime();
		const auto& location = getLocation();
		const auto& dir = getDirection();

		auto cacheOrm = cacheTx->acquireObject(*player, _playerDb);

		cacheOrm->setLastExitTime(curTime);
		cacheOrm->setYaw(dir.getYaw());

		if (auto world = getWorld())
		{
			if (world->getInfo().getIsVillage())
			{
				cacheOrm->setX(location.x);
				cacheOrm->setY(location.y);
				cacheOrm->setZ(location.z);
				cacheOrm->setCurWorldInfoId(world->getInfoId());
			}
			else
				if (_lastPublicWorldInfoId)
				{
					cacheOrm->setCurWorldInfoId(_lastPublicWorldInfoId);
					cacheOrm->setCurWorldInfoId(_lastPublicWorldInfoId);
					cacheOrm->setX(_lastPublicWorldLocation.x);
					cacheOrm->setY(_lastPublicWorldLocation.y);
					cacheOrm->setZ(_lastPublicWorldLocation.z);
				}
		}

		cacheOrm->updateCache();

		cacheTx->ifSucceed(*player, [player]()
			{
				INFO_LOG("user disconnect player save!! [pid:%llu]", player->getId());
				player->leaveWorld(nullptr);
				player->finalize();
			});
		cacheTx->ifFailed(*player, [player]()
			{
				WARN_LOG("user disconnect player save failed!! [pid:%llu]", player->getId());
				player->leaveWorld(nullptr);
				player->finalize();
			});

		cacheTx->run();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  디스커넥을 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::disconnnectSave(RoomThreadContextRef)
{
	if (isWorldMoveLoading())
	{
		setWorldMoveLoading(false);
		updateSight();
	}

	if (auto player = shared_from_this<Player>())
	{
		auto cacheTx = std::make_shared<CacheTx>(Caller);
		
		auto curTime = Core::Time::GetCurTime();
		const auto& location = getLocation();
		const auto& dir = getDirection();

		auto cacheOrm = cacheTx->acquireObject(*player, _playerDb);

		cacheOrm->setLastExitTime(curTime);
		cacheOrm->setYaw(dir.getYaw());

		if (auto world = getWorld())
		{
			if (world->getInfo().getIsVillage())
			{
				cacheOrm->setX(location.x);
				cacheOrm->setY(location.y);
				cacheOrm->setZ(location.z);
				cacheOrm->setCurWorldInfoId(world->getInfoId());
			}
			else
				if (_lastPublicWorldInfoId)
				{
					cacheOrm->setCurWorldInfoId(_lastPublicWorldInfoId);
					cacheOrm->setCurWorldInfoId(_lastPublicWorldInfoId);
					cacheOrm->setX(_lastPublicWorldLocation.x);
					cacheOrm->setY(_lastPublicWorldLocation.y);
					cacheOrm->setZ(_lastPublicWorldLocation.z);
				}
		}

		cacheOrm->updateCache();


		cacheTx->ifSucceed(*player, [player]()
			{
				INFO_LOG("user disconnect player save!! [pid:%llu]", player->getId());
			});
		cacheTx->ifFailed(*player, [player]()
			{
				WARN_LOG("user disconnect player save failed!! [pid:%llu]", player->getId());
			});
		cacheTx->run();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  룸스레드에서 작업을 수행한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::runTask(const Core::TaskCaller& taskCaller, const Core::TaskManager::Task& task)
{
	if (auto taskManager = getTaskManager())
		taskManager->runTask(taskCaller, task);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 갱신한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::update(int64_t curTime)
{
	if (auto user = getUser())
		user->consumePacketQueue();

	super::update(curTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		작업관리자를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
Core::TaskManagerPtr Player::getTaskManager()
{
	auto room = getRoom();
	return room ? room->getTaskManager() : nullptr;
}

Vector Player::getDBLocation() const
{
	return _playerDb ? _playerDb->getLocation() : getLocation();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  닉네임을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string& Player::getNick() const
{
	static std::string emptyStr;
	return _playerDb ? _playerDb->getNick() : emptyStr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
int32_t Player::getCurWorldInfoId() const
{
	if (!_playerDb)
		return 0;

	return _playerDb->getCurWorldInfoId();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::setCurWorldInfoId(int32_t infoId)
{
	if (_playerDb)
		_playerDb->setCurWorldInfoId(infoId);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷정보를 내보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::exportTo(PktPlayerData& dest) const
{
	dest.setId(_id);
	dest.setLocation(_location);
	dest.setFoward(_direction);
	dest.setRepresentCharacterId(_playerDb ? _playerDb->getRepresentCharacterId() : 0);
	dest.setCurWorldInfoId(getCurWorldInfoId());
	dest.setNick(getNick());

	{
		const auto& comp = getCharacterComponent();
		
		comp.exportTo(dest.getCurrencies());
		comp.exportTo(dest.getCharacters());
		comp.exportTo(dest.getCharacterSlots());
	}
	
	{
		const auto& comp = getEquipComponent();
	}
	{
		const auto& comp = getInvenComponent();
		comp.exportTo(dest.getItems());
	}
	{
		const auto& comp = getQuestComponent();
		comp.exportTo(dest.getCurQuests());
	}
	{
		const auto& comp = getAchievementComponent();
		comp.exportTo(dest.getAchievements());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	패킷정보를 내보낸다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::exportTo(PktSightEnterNotify& dest) const
{
	PktPlayerSightInfo sightInfo;
	exportTo(sightInfo);

	dest.getPlayerSightInfos().emplace_back(std::move(sightInfo));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  시야정보에 대해서 내보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::exportTo(PktPlayerSightInfo& dest) const
{
	dest.setId(getId());
	dest.setLocation(_location);
	dest.setFoward(_direction);
	dest.setRepresentCharInfoId(_info ? _info->getId() : 0);
	dest.setNick(getNick());

	{
		const auto& comp = getCharacterComponent();
	}

	{
		auto& comp = getEquipComponent();
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  방에서 나가기 전 호출
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::endPlay()
{
	super::endPlay();

	if (auto world = getWorld())
	{
		INFO_LOG("player leave world [pid:%llu, worldId:%llu]", getId(), world->getId());

		if (world->getInfo().getIsVillage())
		{
			setLastPublicWorldInfoId(world->getInfoId());
			setLastPublicWorldLocation(getLocation());
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  방에서 삭제된다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::endPlayPost()
{
	ENSURE(!getRoom(), return);

	runRoomLeaveCallback();
}

void Player::beginPlay()
{
	super::beginPlay();

	if ( auto world = getWorld())
		INFO_LOG("player enter world [pid:%llu, worldId:%llu]", getId(), world->getId());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		보고 있는지 여부를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Player::isSeeingActorId(PktObjId id) const
{
	return _seeingActorIds.find(id) != _seeingActorIds.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		보고 있는지 여부를 추가한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::addSeeingActorId(PktObjId id)
{
	if (getId() == id)
		return;

	_seeingActorIds.insert(id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		보고 있는지 여부를 제거한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::removeSeeingActorId(PktObjId id)
{
	_seeingActorIds.erase(id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		보고 있는걸 전부 제거한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::removeSeeingAll()
{
	_seeingActorIds.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  보여지는 숫자를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
size_t Player::getSeeingCount() const
{
	return _seeingActorIds.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  루트포지션을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
Vector Player::getRootLocation() const
{
	ENSURE(_info, return _location);

	return _location - Vector(0, 0, (float)(_info->getCapsuleHalfHeight()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Busy처리기를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
const BusyStateUpdaterPtr Player::createBusyUpdater() const
{
	BusyStateUpdaterPtr busyUpdater;
	if (auto user = getUser())
		busyUpdater = user->createBusyUpdater();

	return busyUpdater;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc를 Kill 한것을 전파한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::broadcastKillNpc(CacheTx& cacheTx, Npc& npc)
{
	if (!checkThread())
		return;

	_killNpcDelegate.broadcast(cacheTx, *this, npc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템을 장착한것을 전파한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::broadcastEquipItem(CacheTx& cacheTx, Item& item)
{
	if (!checkThread())
		return;

	_equipOnDelegate.broadcast(cacheTx, *this, item);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  아이템 획득을 전파한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::broadcastAcquireItem(CacheTx& cacheTx, const ItemInfo& info)
{
	if (!checkThread())
		return;

	_acquireItemDelegate.broadcast(cacheTx, *this, info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc인터렉션을 전파한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::broadcastNpcInteraction(CacheTx& cacheTx, const NpcInfo& info, EIfCondition conditionType)
{
	if (!checkThread())
		return;

	_npcInteractionDelegate.broadcast(cacheTx, *this, info, conditionType);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  트리거 진입을 전파한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::broadcastTriggerEnter(const Trigger& trigger)
{
	if (!checkThread())
		return;

	if (!_triggerEnterDelegate.empty())
	{
		CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
		_triggerEnterDelegate.broadcast(*cacheTx, *this, trigger);

	}
}
