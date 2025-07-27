////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		룸 클래스
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <memory>

#include "Actor/Actor.h"
#include "Actor/ActorTypes.h"
#include "CellManager.h"
#include "Component/Entity.h"
#include "RoomThread.h"
#include "RoomThreadContext.h"

class Player;
class RoomThread;
class PktBase;
class PktSightEnterNotify;
class PktSightLeaveNotify;


class Room
	:
	public std::enable_shared_from_this<Room>,
	public Entity
{
protected:
	/// 플레이어 콜백 타입 정의
	using SendExceptCallback = std::function<bool(const PlayerPtr&)>;
	using Task = std::function<void(RoomThreadContextRef)>;

protected:
	uint64_t             _id;            ///< 식별자
	RoomThread&          _thread;        ///< 스레드
	bool				 _removable;     ///< 제거될 지 여부
	std::atomic_int		 _playerCount;	 ///< 플레이어 수
	ActorMap			 _actors;	     ///< 액터 목록
	PlayerMap			 _players;	     ///< 플레이어 목록
	NpcMap              _npcs;          ///< 엔피시 목록
	Core::TaskManagerPtr _taskManager;	 ///< 작업 관리자
	int64_t              _time;          ///< 월드시간
	int64_t              _createTime;    ///< 월드 생성시간
	CellManager			 _cellManager;	 ///< 셀 관리자
	int64_t              _forceUpdateCheckTime; ///< 업데이트 체크용 시간
	std::atomic_int      _leaveReserveCount; ///< 나가기예약카운트
	std::atomic_int      _enterReserveCount; ///< 진입예약카운트
	int32_t              _deltaMsec; ///< 프레임밀리초
	float                _deltaSec; ///< 프레임초
	std::unordered_map<std::string, ActorWeakMap> _subscrives; ///< 채널구독액터목록
public:
	/// 생성자
	Room(RoomThread& thread);

	/// 소멸자
	virtual ~Room();

	/// 제거될 지 여부를 설정한다
	void setRemovable(bool removeable) { 
		_removable = removeable; }

	/// 업데이트 항상 되어야 하는 시간체크를 설정한다
	void setForceUpdateCheckTime(int64_t t) {
		_forceUpdateCheckTime = t; }

	/// 식별자를 반환한다
	uint64_t getId() const { 
		return _id; }

	/// 월드시간을 반환한다
	int64_t getTime() const { 
		return _time; }

	/// 월드 시간을 설정한다.
	void setTime(int64_t t) {
		_time = t;
	}

	/// 프레임밀리초를 설정한다.
	void setDeltaMSec(int32_t msec) {
		_deltaMsec = msec;
		_deltaSec = msec / 1000.f;
	}

	/// 가져온다.
	int32_t getDeltaMSec() const {
		return _deltaMsec;
	}
	float getDeltaSec() const {
		return _deltaSec;
	}

	/// 플레이어 수를 반환한다
	int32_t getPlayerCount() const { 
		return _playerCount.load(); }

	/// 제거될 지 여부를 반환한다
	bool isRoomRemovable() const { 
		return _removable; }

	/// 스레드 식별자를 반환한다
	const std::thread::id& getThreadId() const {
		return _thread.getId(); }

	const PlayerMap& getPlayerList() const { 
		return _players; }

	/// 엔피시 목록을 반환한다.
	const NpcMap& getNpcList() const { 
		return _npcs; }

	/// 셀 매니저를 반환한다
	CellManager& getCellManager() { 
		return _cellManager; }

	/// 셀 매니저를 반환한다
	const CellManager& getCellManager() const { 
		return _cellManager; }

	/// 플레이어를 반환한다
	const PlayerPtr getPlayer(PktObjId playerId) const;

	/// Npc를 반환한다.
	const NpcPtr getNpc(PktObjId npcId) const;

	/// 나가기 예약카운트를 증가한다
	void increaseLeaveReserveCount() {
		_leaveReserveCount.fetch_add(1);
	}
	/// 나가기 예약카운트를 감소한다
	void decreaseLeaveReserveCount() {
		_leaveReserveCount.fetch_sub(1);
	}
	/// 나가기 예약카운트를 반환한다.
	int32_t getLeaveReserveCount() const {
		return _leaveReserveCount.load();
	}
	/// 진입 예약 카운트를 증가시킨다.
	void increaseEnterReserveCount() {
		_enterReserveCount.fetch_add(1);
	}
	/// 진입 예약카운트를 감소한다
	void decreaseEnterReserveCount() {
		_enterReserveCount.fetch_sub(1);
	}
	/// 진입 예약카운트를 반환한다.
	int32_t getEnterReserveCount() const {
		return _enterReserveCount.load();
	}

	/// 대상셀이 소스셀의 인접셀인지 여부를 반환한다.
	bool isNearChellNo(int32_t src, int32_t dest) const;
public:
	/// 초기화 한다
	virtual void initialize();

	/// 해제한다.
	virtual void finalize();

	/// 스레드가 올바른지 체크한다
	virtual bool checkThread() const;

	/// 액터를 추가한다
	bool addActor(ActorPtr actor);

	/// 액터를 찾는다
	const ActorPtr findActor(PktObjId objId) const;

	/// 액터를 찾는다
	template<typename T>
	std::shared_ptr<T> findActor(PktObjId objId);

	/// 패킷을 보낸다.
	void sendToAll(PktBase& packet) const;

	/// 패킷을 보낸다.
	void sendToAllExcept(const Actor& actor, PktBase& packet) const;

	/// 패킷을 보낸다.
	void sendToAllExcept(PktBase& packet, const SendExceptCallback& exceptfunc) const;

	/// 패킷을 보낸다.
	void sendToNear(const Actor& actor, PktBase& packet) const;

	/// 패킷을 보낸다.
	void sendToNearExcept(const Actor& actor, PktBase& packet) const;

	/// 플레이어를 순회한다
	void forEachPlayer(const PlayerCallback& callback) const;

	/// 액터를 순회한다
	void forEachActor(int32_t cellNo, const ActorCallback& callback) const;

	/// 플레이어를 순회한다
	void forEachPlayer(int32_t cellNo, const PlayerCallback& callback) const;

	/// Npc를 순회한다
	void forEachNpc(int32_t cellNo, const NpcCallback& callback) const;

	/// 룸에서 작업을 한다.
	void runTask(const Core::TaskCaller& taskCaller, const Task& task);

	/// 룸에서 작업을 한다.(동일스레드면 바로)
	void dispatchTask(const Core::TaskCaller& taskCaller, const Task& task);

	/// 출판한다.
	void setPublish(const std::string& channelName, const std::string& message);

	/// 구독한다.
	void setSubscribe(Actor* actor, const std::string& channelName);

	/// 구독을 취한다.
	void cancelSubscrive(Actor* actor, const std::string& channelName);

public:
	/// 갱신한다
	virtual void update();
	

public:
	/// 작업 관리자를 반환한다
	Core::TaskManagerPtr getTaskManager();

private:
	/// 액터를 제거한다
	void _removeActor(Actor* actor);

protected:
	void _updateSight(Actor& actor, const CellNos* cellNos = nullptr, bool isSendedSpawnNoti = false);
	friend void Actor::updateSight(const CellNos*, bool); ///< 액터의 updateSight friend설정
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	액터를 형변환하여 가져온다
////////////////////////////////////////////////////////////////////////////////////////////////////
template< typename T >
std::shared_ptr< T > Room::findActor(PktObjId objId)
{
	auto actor = findActor(objId);
	if (!actor)
		return nullptr;

	return std::dynamic_pointer_cast<T>(actor);
}
