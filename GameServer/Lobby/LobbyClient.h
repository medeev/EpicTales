////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		로비서버 클라이언트
///
///	@ date		2024-3-7
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Asio/Service/Client.h>

#include "Packet/L2GPacketDispatcher.h"

class PktBase;
class GameServer;

class LobbyClient
	:
	public Core::Client
{
private:
	GameServer*             _gameServer; ///< 게임 서버
	L2GPacketDispatcher	    _dispatcher;///< 패킷 디스패처
	bool					_tryConnect;///< 접속 시도중인지 여부
	std::string				_ip;		///< 아이피
	uint16_t				_port;		///< 포트
	uint32_t				_numbering;	///< 넘버링

public:
	/// 생성자
	LobbyClient(AsioIoContext& ioContext, GameServer* gameServer);

	/// 소멸자
	virtual ~LobbyClient();

	/// 시작한다
	void start(const std::string& ip, uint16_t port);

	/// 패킷을 보낸다
	bool send(const PktBase& pktBase);

private:
	/// 접속 성공
	virtual void onConnected() override;

	/// 접속 실패
	virtual void onConnectFailed(const AsioErrorCode& error) override;

	/// 전송 성공
	virtual void onSent(int32_t bytesTransferred) override;

	/// 읽기 완료 후 패킷 구분 시 호출된다
	virtual bool onExtractPacket(uint8_t* data, uint32_t len, uint32_t& outLen) override;

	/// 패킷을 디스패치 한다
	virtual void onPacketDispatch(uint8_t* data) override;

	/// 연경 종료
	virtual void onClosed() override;

};
