
#include "Pch.h"
#include "Actor.h"

#include <Core/Math/Numeric.h>
#include <Core/Math/Vector2.h>
#include <Data/Collision/CircleUnitInfo.h>
#include <Data/Collision/Unit/CollisionUnit.h>
#include <Data/Info/InfoEnumsConv.h>
#include <Protocol/PktCommon.h>
#include <Protocol/Struct/PktActor.h>
#include <Protocol/Struct/PktShape.h>

#include "Player.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
Actor::Actor(EActorType type)
	:
	_type(type),
	_id(0),
	_isRoomRemovable(false),
	_isWorldMoveLoad(false),
	_room(nullptr),
	_cellNo(-1),
	_direction(1, 0, 0),
	_lifeEndTime(0),
	_stateType(EFsmStateType::StateNone),
	_haveTokens((int32_t)EPosTokenType::Max),
	_teamType(ETeamType::Max),
	_fsmComponent(*this)
{

}

void Actor::initialize()
{
	initComponents();
}

void Actor::finalize()
{
	finalizeComponents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  룸에 입장후에 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::beginPlay()
{
	auto room = getRoom();
	ENSURE(room, return);

	initLocationToken();
	componentBeginPlay();
	room->getCellManager().onCellChange(*this);
}

bool Actor::checkThread() const
{
	if (auto room = getRoom())
		return room->checkThread();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  룸에서 삭제되기전 호출된다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::endPlay()
{
	auto room = getRoom();
	ENSURE(room, return);

	clearSubscrive();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	볼수 있는지 여부를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Actor::canSee(const Actor& target) const
{
	auto room = getRoom();
	ENSURE(room, return false);

	if (target.getRoom() != room)
		return false;

	if (_isRoomRemovable)
		return false;
	if (target._isRoomRemovable)
		return false;
	if (_isWorldMoveLoad)
		return false;
	if (target._isWorldMoveLoad)
		return false;

	return room->isNearChellNo(_cellNo, target.getCellNo());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		대상과의 거리를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
float Actor::getDistance(const Actor& target) const
{
	float dist = getLocation().getDistance(target.getLocation());
	if (dist <= 0.f)
		dist = 0.f;

	return dist;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	대상과의 2d거리를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
float Actor::getDistance2D(const Actor& target) const
{
	float dist = getLocation().getDistance2D(target.getLocation());
	if (dist <= 0.f)
		dist = 0.f;

	return dist;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief 대상과의 제곱된 거리를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
float Actor::getDistanceSqr(const Actor& target) const
{
	return _location.getSquaredDistance(target.getLocation());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	대상과의 2D거리 제곱을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
float Actor::getDistanceSqr2D(const Actor& target) const
{
	return _location.getSquaredDistance2D(target.getLocation());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	시야를 갱신한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::updateSight(const CellNos* cellNos, bool isSendedSpawnNoti)
{
	if (auto room = getRoom())
		room->_updateSight(*this, cellNos, isSendedSpawnNoti);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  위치를 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::setLocation(const Vector& location)
{
	if (_location == location)
		return;

	auto preLocation = _location;
	_location = location;

	if (auto room = getRoom())
	{
		if (!room->getCellManager().onCellChange(*this))
		{
			onRoomInvalidCellBound(location);
			_location = preLocation;
			return;
		}
	}

	if (_collider)
		_collider->setPosition(location);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  방향을 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::setDirection(const Vector& dir)
{
	_direction = dir;

	if (_collider)
		_collider->setRotate(Core::Rotator(0, 0, dir.getYaw()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		룸을 설정한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::setRoom(Room* room)
{
	_room = room;
	_roomWeakPtr = room ? room->shared_from_this() : nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		월드를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
const WorldPtr Actor::getWorldPtr() const
{
	auto room = getRoomPtr();
	if (!room)
		return nullptr;

	return std::dynamic_pointer_cast<World>(room);
}

WorldPtr Actor::getWorldPtr()
{
	auto room = getRoomPtr();
	if (!room)
		return nullptr;

	return std::dynamic_pointer_cast<World>(room);
}

const World* Actor::getWorld() const
{
	auto room = getRoom();
	if (!room)
		return nullptr;

	return dynamic_cast<World*>(room);
}

World* Actor::getWorld()
{
	auto room = getRoom();
	if (!room)
		return nullptr;

	return dynamic_cast<World*>(room);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  액터를 업데이트한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::update(int64_t curTime)
{
	updateComponent(curTime);

	if (_lifeEndTime && _lifeEndTime < curTime)
	{
		setRoomRemovable(true);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	주변위치할수있는곳에서 대상위치와 가장 가까운 점유가능한 위치를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Actor::findNearestOccupyToken(
	int64_t curTime,
	const Vector& pos,
	const float acceptRadius,
	Vector& outLocation,
	EPosTokenType& outToken) const
{
	Vector dir = Vector::ForwardVector;

	auto actorLocation = getRootLocation();

	auto radius = acceptRadius * 1.f;

	Vector findLocation = Vector::ZeroVector;
	float findMinDistSqr = Core::Numeric::MaxSingle;
	EPosTokenType findToken = EPosTokenType::None;

	bool ret = false;

	int32_t starti = (int32_t)EPosTokenType::TokenNear0;
	int32_t endi = (int32_t)EPosTokenType::TokenNear11;

	if (radius > 300.f)
	{
		starti = (int32_t)EPosTokenType::TokenFar0;
		endi = (int32_t)EPosTokenType::TokenFar11;

		//dir = dir.rotateAngleAxis( 15.f, Vector::UpVector );
	}

	for (int32_t i = starti; i <= endi; i = i << 1)
	{
		if (hasLocationToken(i))
		{
			auto location = actorLocation + (dir * radius);
			auto distSqr = pos.getSquaredDistance(location);

			if (findMinDistSqr > distSqr)
			{
				findMinDistSqr = distSqr;
				findLocation = location;
				findToken = (EPosTokenType)i;
				ret = true;
			}
		}

		dir = dir.rotateAngleAxis(30.f, Vector::UpVector);
	}

	if (ret)
	{
		outLocation = findLocation;
		outToken = findToken;
	}

	return ret;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	주변위치할수있는곳에서 대상위치와 가장 가까운 점유가능한 위치를 반환한다(내비패스체크포함) 미완성!!
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::getTokenPosition(
	const float acceptRadius,
	bool isHave,
	std::vector<Vector>& outPositions) const
{
	Vector dir = Vector::ForwardVector;

	auto actorLocation = getRootLocation();
	auto radius = acceptRadius * 1.f;

	int32_t starti = (int32_t)EPosTokenType::TokenNear0;
	int32_t endi = (int32_t)EPosTokenType::TokenNear11;

	if (radius > 300.f)
	{
		starti = (int32_t)EPosTokenType::TokenFar0;
		endi = (int32_t)EPosTokenType::TokenFar11;

		//dir = dir.rotateAngleAxis(15.f, Vector::UpVector);
	}

	for (int32_t i = starti; i <= endi; i = i << 1)
	{
		if (hasLocationToken(i) == isHave)
		{
			auto location = actorLocation + (dir * radius);
			outPositions.push_back(location);
		}

		dir = dir.rotateAngleAxis(30.f, Vector::UpVector);
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	충돌 유닛을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
CollisionUnit* Actor::getCollisionUnit(
	CollisionUnitBuffer& buffer,
	const CollisionUnitInfo* collisionUnitInfo,
	float offsetDistance,
	float offsetAngle, 
	float collisionYaw)
{
	if (!collisionUnitInfo)
		return nullptr;

	CollisionUnit* collisionUnit = CollisionUnit::Create(*collisionUnitInfo, buffer);

	Core::Vector2 pos(getLocation());
	auto dir = getDirection();
	if (offsetDistance)
	{
		Core::Vector2 collisionDir = Core::Vector2::Rotate(Core::Vector2(dir), Core::Numeric::ToRadian(offsetAngle));
		collisionDir.normalize();
		pos = pos + collisionDir * offsetDistance;
	}
	
	if (offsetAngle + collisionYaw)
	{
		auto dir2d = Core::Vector2::Rotate(Core::Vector2(dir), Core::Numeric::ToRadian(offsetAngle + collisionYaw));
		dir = Vector(dir2d.x, dir2d.y, 0);
	}

	collisionUnit->setPos(pos);
	collisionUnit->setDir(dir);
	//collisionUnit->increaseRadius(getRadius());

	return collisionUnit;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	충돌 유닛을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
CollisionUnit* Actor::getCollisionUnit(CollisionUnitBuffer& buffer) const
{
	CircleUnitInfo circleUnitInfo;
	circleUnitInfo.radius = getRadius();
	CollisionUnit* collisionUnit = CollisionUnit::Create(circleUnitInfo, buffer);
	collisionUnit->setPos(getLocation());
	collisionUnit->setDir(getDirection());
	return collisionUnit;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  채널에 출판한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::setPublish(const std::string& channelName, const std::string& message)
{
	auto room = getRoom();
	ENSURE(room, return);

	room->setPublish(channelName, message);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  채널을 구독한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::setSubscribe(const std::string& channelName)
{
	auto room = getRoom();
	ENSURE(room, return);

	auto& channelMessage = _subscribeRecieveMsgs[channelName];
	room->setSubscribe(this, channelName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  모든 구독을 클리어한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::clearSubscrive()
{
	auto room = getRoom();
	ENSURE(room, return);

	for (const auto& channel : _subscribeRecieveMsgs)
		room->cancelSubscrive(this, channel.first);
	_subscribeRecieveMsgs.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  채널로부터 해당 메시지를 수신했는지 여부를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Actor::popChannelMessage(
	const std::string& channelName, const std::string& message)
{
	auto iter = _subscribeRecieveMsgs.find(channelName);
	if (iter == _subscribeRecieveMsgs.end())
	{
		WARN_LOG("not subscrive channel [name:%s]", channelName.c_str());
		return false;
	}

	auto& channelMessage = iter->second;
	auto iterMessage = channelMessage.find(message);
	if (iterMessage == channelMessage.end())
		return false;

	channelMessage.erase(iterMessage);
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  채널로부터 메시지를 수신한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Actor::reciveChannelMessage(const std::string& channelName, const std::string& message)
{
	auto iter = _subscribeRecieveMsgs.find(channelName);
	if (iter == _subscribeRecieveMsgs.end())
	{
		WARN_LOG("recv. but not subscrive channel [name:%s]", channelName.c_str());
		return;
	}

	auto& channelMessage = iter->second;
	channelMessage.insert(message);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  대상이 공격가능한지 판단한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
ETeamRelation Actor::getRelation(Actor& target) const
{
	return ETeamRelation::Friend;
}
