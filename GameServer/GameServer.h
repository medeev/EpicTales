////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	GameServer 클래스
///
/// @date	2024-3-7
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "User/UserManager.h"
#include "Lobby/LobbyClient.h"
#include "Logic/Party/PartyManager.h"
#include <Asio/Service/BaseServer.h>
#include <Core/Task/TimerTask.h>

class GameServer
	:
	public Core::BaseServer,
	public Core::Singleton<GameServer>
{
	typedef Core::BaseServer super;
	friend class Core::Singleton<GameServer>;

private:
	LobbyClient _lobbyClient; ///< 로비 클라이언트	

private:
	/// 생성자
	GameServer();

	/// 소멸자
	virtual ~GameServer();

public:

	/// 로비와 접속디 되면 처리한다.
	void onAccepterStart();

protected:
	virtual void _onStartup() override;
	virtual void _onCleanup() override;

protected:

};

