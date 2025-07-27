
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		유저 클래스
///
///	@ date		2024-3-6 
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "UserTypes.h"
#include "Actor/ActorTypes.h"
#include "DB/BusyState.h"
#include "Room/RoomTypes.h"
#include "Packet/GamePacketDispatcher.h"

#include <fstream>
#include <Asio/Service/Connection.h>
#include <Core/Thread/LockFreeQueue.h>
#include <Protocol/PacketWriter.h>
#include <Data/Info/InfoEnums.h>


namespace Core {
	class NetBuffer;
}

class NaglePacket {
public:
	PacketWriterPtr writer;	///< 패킷 라이터
	NaglePacket(const PacketWriterPtr& _writer)
		:
		writer(_writer)
	{

	}
};

class NaglePacketPack {
public:
	Core::BoostLFQ<NaglePacket*> queue; ///< 전송 큐
	std::atomic_int64_t totalSize; ///< 전체 사이즈
	std::atomic_bool processed;	///< 완료 여부
	NaglePacketPack() 
		:
		totalSize(0),
		processed(false),
		queue(0)
	{

	}
};

/// 네이글 패킷 포인터 타입 정의
typedef std::shared_ptr< NaglePacketPack > NaglePacketPackPtr;

class User
	:
	public Core::Connection
{
	typedef Core::Connection super;

	/// 패킷 큐 콜백 타입 정의
	typedef std::function< void(PktBase*) > PacketQueueCallback;

private:
	UserWeakPtr          _userWeakPtr;        ///< 약참조유저
	uint64_t             _id;                 ///< 식별자
	uint32_t             _packetVersion;      ///< 패킷버전
	uint16_t             _numbering;          ///< 패킷 넘버링
	GamePacketDispatcher _dispather;          ///< 디스패처
	PlayerWeakPtr        _playerWeakPtr;      ///< 플레이어 위크 포인터
	Core::BoostLFQ<PktBase*> _packetQueue; ///< 패킷 큐
	Core::BoostLFQ<PktBase*> _packetQueueForNotCheckBusy; ///< Busy체크 필요없는패킷큐
	PacketQueueCallback  _packetQueueDispose; ///< 패킷 큐 처리자
	bool                 _versionCheck;       ///< 버전 체크 여부
	UserWaitRoomWeakPtr  _waitRoom;           ///< 대기 룸
	int64_t              _lastPingTime;       ///< 마지막 핑시간
	int32_t              _latency;            ///< 레이턴시
	int32_t              _latencyCount;       ///< 레이턴시 저장 횟수
	int64_t              _latencyCheckTime;   ///< 레이턴시 체크시작시간
	std::string          _account;            ///< 계정
	std::atomic<NaglePacketPackPtr> _naglePacket; ///< 네이글 패킷
	BusyStatePtr         _busyState;
	bool _isCleanCloseData; ///< 종료시 데이터 처리
	bool _isCustomNagle; ///< 커스텀 네이글 사용(룸에 진입시)
	bool _isCloseProcessed;
	std::ofstream _packetFile; ///< 패킷 파일 저장용

public:
	/// 생성자
	User(int32_t connectionId);

	/// 소멸자
	virtual ~User();

	/// 패킷큐를 반환한다.
	Core::BoostLFQ<PktBase*>& getPacketQueue(PktBase& packet);

	/// 식별자를 반환한다
	uint64_t getId() const { 
		return _id; }

	/// 식별자를 설정한다
	void setId(uint64_t id) {
		_id = id; }

	/// 버전을 설정한다.
	void setPacketVersion(uint32_t packetVersion) {
		_packetVersion = packetVersion;
	}

	/// 계정을 설정한다.
	void setAccount(const std::string& account) {
		_account = account;
	}
	/// 계정을 반환한다.
	const std::string& getAccount() const {
		return _account;
	}

	void setUserPtr(UserPtr user) {
		_userWeakPtr = user;
	}
	UserPtr getUserPtr() {
		return _userWeakPtr.lock();
	}

	/// 플레이어를 설정한다
	void possess(PlayerPtr player);
	void unPossess();

	/// 버전 체크 여부를 설정한다
	void setVersionCheck() { 
		_versionCheck = true; };

	/// 버전 체크 여부를 반환한다
	bool isVersionCheck() { 
		return _versionCheck; }

	/// check keepalive
	void checkKeepAlive(int64_t curTime);

	/// 핑체크 시간을 설정한다.
	void setPingTime(int64_t pingTime) { 
		_lastPingTime = pingTime; }

	/// 평균 레이턴시를 설정한다.
	void setLatency(int32_t latency)
	{
		if (_latencyCount)
		{
			auto value = _latencyCount * _latency;
			value += latency;

			latency = (value / (_latencyCount + 1));
		}

		_latency = latency;
		++_latencyCount;
	}

	/// 레이턴시를 설정한다.
	int32_t getLatency() const {
		return _latency;
	}
	/// 레이턴체크시작시간을 가져온다.
	int64_t getLatencyCheckTime() const {
		return _latencyCheckTime;
	}
	/// 레이턴체크시작시간을 설정한다.
	void setLatencyCheckTime(int64_t t) {
		_latencyCheckTime = t;
	}

	/// 대기 룸을 설정한다
	void setWaitRoom(UserWaitRoomPtr room);

	/// 대기 룸에 입장한다
	void enterWaitRoom();

	/// 대기 룸에서 나온다
	void leaveWaitRoom();

	/// 플레이어를 반환한다
	PlayerPtr getPlayer();

	/// 플레이어를 반환한다
	const PlayerPtr getPlayer() const;

	/// 커스텀 네이글 활성화를 반환한다.
	bool isCustomNagle() const {
		return _isCustomNagle;
	}
	void setCustomNagle(bool value) {
		_isCustomNagle = value;
	}

	/// 로그인이후의 대기방에 있는지 여부를 반환한다.
	bool isWaitingRoom() const {
		return _waitRoom.lock() ? true : false;
	}

	/// Busy 상태처리를 반환한다.
	const BusyStateUpdaterPtr createBusyUpdater() const {
		return std::make_shared<BusyStateUpdater>(_busyState);
	}

	/// 강제로 접속종료 시키고 정리한다.
	void forceClose();

public:
	/// 패킷을 보낸다
	bool send(const PktBase& pktBase);

	/// 네이글 패킷을 보낸다
	void sendNaglePacket();

	/// 결과 패킷을 보낸다
	template< typename T >
	void sendResultCode(EResultCode resultCode)
	{
		T pktResult;
		pktResult.setResult(resultCode);
		send(pktResult);
	}

	/// 패킷을 처리한다
	void consumePacketQueue();

protected:
	/// 패킷을 추출한다.
	virtual uint32_t _onExtractPacket(uint8_t* data, uint32_t len, bool& result) override;

	/// 패킷을 디스패치한다.
	virtual void _onPacketDispatch(Core::NetBuffer& buffer) override;

	/// 패킷을 디스패치한다.
	void _onPacketDispatchInRoom(PacketReader& pktReader, int16_t packetId);

	/// 연결을 처리한다.
	virtual void _onConncted() override;

	/// 연결 종료를 처리한다.
	virtual void _onClosed() override;

	/// 쓰기가 시작됐을 때를 처리한다.
	virtual void _onWriteStarted(int32_t /*bytesTransferred*/) {}

	/// 쓰기가 완료됐을 때를 처리한다.
	virtual void _onWriteCompleted(int32_t /*bytesTransferred*/) {}

	/// Close시 처리한하는
	void _closeProcess();

private:
	/// 로그를 기록한다
	void _dispatchLog(const PktBase& pktBase) const;

public:
	/// 유저를 반환한다
	static UserPtr Get(uint64_t id);
};

