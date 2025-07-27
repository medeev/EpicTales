////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		액터
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Core/Collision/Collider.h>
#include <Data/Info/InfoEnums.h>
#include <Protocol/Struct/PktTypes.h>

#include "ActorTypes.h"
#include "Component/Entity.h"
#include "Logic/Counts/ActorStat.h"
#include "Logic/Fsm/FsmComponent.h"
#include "Logic/Fsm/FsmTypes.h"
#include "Logic/Npc/ActorMoveEnum.h"
#include "Room/Cell.h"
#include "Room/RoomTypes.h"

class EffectApply;
class PktSightEnterNotify;
class WorldInfo;
class Trigger;
class CollisionUnit;
class CollisionUnitBuffer;
class CollisionUnitInfo;
class Actor 
	: public Entity
{
protected:
	PktObjId     _id;              ///< 식별자
	ActorWeakPtr _weakPtr;		   ///< 위크 포인터
	RoomWeakPtr  _roomWeakPtr;	   ///< 룸 위크 포인터
	Room*        _room;            ///< 룸 포인터
	bool         _isRoomRemovable; ///< 제거될 지 여부
	bool         _isWorldMoveLoad; ///< 월드이동중
	EActorType   _type;			   ///< 액터 타입
	int32_t		 _cellNo;		   ///< 셀 번호
	Vector       _location;        ///< 위치
	Vector       _direction;       ///< 정면방향
	int64_t      _lifeEndTime;     ///< 삭제예정시간
	
	int32_t      _haveTokens;      ///< 주변의 위치점유플래그
	EFsmStateType _stateType;      ///< 현재State
	ETeamType _teamType;

	Core::Collision::ColliderPtr _collider; ///< 컬리젼

	FsmComponent _fsmComponent;

	std::unordered_map<std::string, std::multiset<std::string>> _subscribeRecieveMsgs; ///< 구독중인 채널과 채널에서 온 메시지목록

public:
	/// 생성자
	Actor(EActorType type);

	/// 초기화 한다
	virtual void initialize();

	/// 해제한다.
	virtual void finalize();

	/// 상태머신을 반환한다.
	FsmComponent& getFsmComponent() {
		return _fsmComponent;
	}
	const FsmComponent& getFsmComponent() const {
		return _fsmComponent;
	}
	

	/// 스레드를 체크한다.
	bool checkThread() const;

	/// 식별자를 설정한다
	void setId(PktObjId id) {
		_id = id;
	}

	/// 식별자를 반환한다
	const PktObjId getId() const {
		return _id;
	}

	template<typename T>
	T* cast() {
		return dynamic_cast<T*>(this);
	}

	/// 제거될 지 여부를 반환한다
	bool isRoomRemovable() const {
		return _isRoomRemovable;
	}

	/// 제거될 지 여부를 설정한다
	void setRoomRemovable(bool isRemoveRoom) {
		_isRoomRemovable = isRemoveRoom;
	}

	/// 월드이동중여부
	bool isWorldMoveLoading() const {
		return _isWorldMoveLoad;
	}

	/// 월드이동중여부
	void setWorldMoveLoading(bool value) {
		_isWorldMoveLoad = value;
	}

	/// 액터 타입을 반환한다
	const EActorType getType() const {
		return _type;
	}

	/// 쉐어드 포인터를 설정한다
	void setPtr(ActorPtr ptr) {
		_weakPtr = ptr;
	}

	/// 쉐어드 포인터를 반환한다
	ActorPtr shared_from_this() const {
		return _weakPtr.lock();
	}

	template<typename T>
	std::shared_ptr<T> shared_from_this() {
		auto ptr = _weakPtr.lock();
		return std::dynamic_pointer_cast<T>(ptr);
	}

	/// 룸을 반환한다
	Room* getRoom() const {
		return _room;
	}
	RoomPtr getRoomPtr() const {
		return _roomWeakPtr.lock();
	}

	/// 방에 진입후 호출된다.
	virtual void beginPlay();

	/// 방에서 나가기전 호출된다.
	virtual void endPlay();

	/// 방에서 나가고 호출된다.
	virtual void endPlayPost() {}

	/// 유효하지 않은 셀영역에 진입한다.
	virtual void onRoomInvalidCellBound(const Vector& location) {}

	/// 트리거 진입을 전파한다.
	virtual void broadcastTriggerEnter(const Trigger& trigger) {}


	/// 캡슐 반지름을 구한다.
	virtual float getRadius() const = 0;
	virtual float getHalfHeight() const = 0;
	const ETeamType getTeamType() const {
		return _teamType;
	}

	/// 루트포지션을 구한다(바닥)
	virtual Vector getRootLocation() const {
		return getLocation();
	}

	/// 볼수 있는지 여부를 반환한다
	virtual bool canSee(const Actor& target) const;

	/// 거리를 구한다.
	float getDistance(const Actor& target) const;

	/// 2D 거리를 구한다.
	float getDistance2D(const Actor& target) const;

	/// 거리 제곱을 구한다.
	float getDistanceSqr(const Actor& target) const;

	/// 2D 거리 제곱을 구한다.
	float getDistanceSqr2D(const Actor& target) const;

	/// 시야를 갱신한다
	void updateSight(const CellNos* cellNos = nullptr, bool isSendedSpawnNoti = false);

	/// 위치를 설정한다.
	void setLocation(const Vector& location);

	/// 위치를 가져온다.
	const Vector& getLocation() const {
		return _location;
	}
	
	/// 방향을 설정한다.
	void setDirection(const Vector& dir);

	/// 방향을 가져온다
	const Vector& getDirection() const {
		return _direction;
	}

	void setCellNo(int32_t no) {
		_cellNo = no;
	}
	int32_t getCellNo() const {
		return _cellNo;
	}

	/// 현재State를 반환한다
	EFsmStateType getState() const {
		return _stateType;
	}

	/// 현재State를 설정한다
	void setState(EFsmStateType state) {
		_stateType = state;
	}

	/// 컬리전을 반환한다
	Core::Collision::ColliderPtr getCollision() const {
		return _collider;
	}

	/// 컬리전을 반환한다
	Core::Collision::ColliderPtr getCollision() {
		return _collider;
	}

	/// Pub/ Sub 기능을 하는 함수목록입니다.
	void setPublish(const std::string& channelName, const std::string& message);
	void setSubscribe(const std::string& channelName);
	void clearSubscrive();
	bool popChannelMessage(const std::string& channelName, const std::string& message);
	void reciveChannelMessage(const std::string& channelName, const std::string& message);

	/// 정보를 내보낸다
	virtual void exportTo(PktSightEnterNotify& dest) const = 0;

	/// 관계를 반환한다.
	ETeamRelation getRelation(Actor& target) const;


public:
	/// 토큰
	void initLocationToken() {
		_haveTokens = (int32_t)EPosTokenType::Max;
	}

	/// 토큰을 추가한다.
	void addLocationToken(int32_t token) {
		_haveTokens |= token;
	}

	/// 토큰을 삭제한다.
	void removeLocationToken(int32_t token) {
		_haveTokens &= ~token;
	}

	/// 위치토큰을 가지고있는지 반환한다.
	bool hasLocationToken(int32_t token) const {
		return (bool)(_haveTokens & token);
	}

	/// 획득가능한 가장 가까운 위치를 얻는다.
	bool findNearestOccupyToken(
		int64_t curTime,
		const Vector& pos,
		const float acceptRadius,
		Vector& outLocation,
		EPosTokenType& outToken) const;

	/// 현재의 토큰 포지션을 반환한다.
	void getTokenPosition(
		const float acceptRadius,
		bool isHave,
		std::vector< Vector >& outPositions) const;

public:
	/// 룸을 설정한다
	void setRoom(Room* room);

	/// 월드를 반환한다
	const WorldPtr getWorldPtr() const;
	WorldPtr getWorldPtr();
	const World* getWorld() const;
	World* getWorld();

	CollisionUnit* getCollisionUnit(
		CollisionUnitBuffer& buffer,
		const CollisionUnitInfo* collisionUnitInfo,
		float offsetDistance,
		float offsetAngle,
		float collisionYaw);
	CollisionUnit* getCollisionUnit(CollisionUnitBuffer& buffer) const;
public:

public:
	/// 갱신한다
	virtual void update(int64_t curTime);


protected:
};
