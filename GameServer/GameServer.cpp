

#include "Pch.h"
#include "GameServer.h"

#include "Admin/AdminManager.h"
#include "Logic/SendThreadManager.h"
#include "Logic/NpcAi/BTNodeManager.h"
#include "Config.h"
#include "Room/RoomThreadManager.h"
#include "Room/WorldManager.h"

#include <Core/Ini.h>
#include <DB/DBTask.h>
#include <DB/Redis/RedisTask.h>
#include <DB/Redis/RedisConn.h>
#include <Core/Task/TimerTask.h>



////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
GameServer::GameServer()
	:
	_lobbyClient(_ioContext, this)
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
GameServer::~GameServer()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화 완료 이벤트
////////////////////////////////////////////////////////////////////////////////////////////////////
void GameServer::onAccepterStart()
{
	static bool isInit = false;
	if (isInit)
		return;

	uint16_t port = Core::Ini::Instance().getInteger("PORT", 6001);
	if (!_acceptor.open(port, &UserManager::Instance(), 1, true))
	{
		WARN_LOG("listen failed");
		return stop();
	}

	VERIFY_LOG("Server is listening %d port.", port);

	isInit = true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		서버가 시작됨을 알린다
////////////////////////////////////////////////////////////////////////////////////////////////////
void GameServer::_onStartup()
{
	super::_onStartup();

	// 1. 타이머 초기화
	Core::TimerTask::Instance().start();
	

	// 2. send스레드 초기화
	SendThreadManager::Instance().initialize();

	// 3. db 초기화
	std::map<EProjDBType, DBConfig> configs;
	{
		DBConfig config;
		config.host = Config::GetLoginDbHost();
		config.dbname = Config::GetLoginDbName();
		config.user = Config::GetLoginDbUser();
		config.password = Config::GetLoginDbPass();

		configs[EProjDBType::Login] = config;
	}
	{
		DBConfig config;
		config.host = Config::GetGameDbHost();
		config.dbname = Config::GetGameDbName();
		config.user = Config::GetGameDbUser();
		config.password = Config::GetGameDbPass();

		configs[EProjDBType::Game] = config;
	}
	DBTask::Instance().initialize(configs);

	// 4. Redis 초기화
	RedisTask::Instance().initialize(Config::GetRedisHost());

	// 5. 룸 초기화
	RoomThreadManager::Instance().initialize(5);

	// 6. 월드 초기화
	WorldManager::Instance().initialize();

	// 7. BehaviorTree 초기화
	BTNodeManager::Instance().initialize();

	// 9. 파티스레드 작동
	PartyManager::Instance().start();


	// 웹서버 시작
	auto server = std::make_shared<HttpServer>(
		_ioContext, tcp::endpoint{ tcp::v4(), (uint16_t)Core::Ini::Instance().getInteger("WEB_PORT", 80) }, handleRequest);
	server->start();

	// 로비서버연결용 클라이언트 시작
	auto lobbyIp = Core::Ini::Instance().getString("LOBBY_SERVER_IP");
	auto lobbyPort = (uint16_t)(Core::Ini::Instance().getInteger("LOBBY_SERVER_PORT"));
	if (lobbyIp.empty() || lobbyPort <= 0)
	{
		WARN_LOG("failed to lobbyServer infomation setting");
		return;
	}

	_lobbyClient.start(lobbyIp, lobbyPort);

	// 킵얼라이브 시작
	if (Config::GetPingKickOut())
	{
		Core::TimerTask::Task task("keepAlive", Core::TimerTask::EType::Loop, 10000, [this](uint64_t curTime)
			{
				UserManager::Instance().foreach([curTime](UserPtr user)
					{
						user->checkKeepAlive(curTime);
					});
			});

		Core::TimerTask::Instance().regist(task, 10000);
	}

	//onAccepterStart();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		서버가 종료됨을 알린다
////////////////////////////////////////////////////////////////////////////////////////////////////
void GameServer::_onCleanup()
{
	super::_onCleanup();

	// 9. 파티스레드를 종료힌다.
	PartyManager::Instance().stop();

	// 7. BehaviorTree 정리한다.
	BTNodeManager::Instance().finalize();

	// 6. 월드 초기화
	WorldManager::Instance().finailze();

	// 5. 룸스레드 정지
	RoomThreadManager::Instance().finalize();

	// 4. Redis 정지
	RedisTask::Instance().finalize();

	// 3. DBThread를 정지한다.
	DBTask::Instance().finalize();

	// 2. SendThread를 정지한다.
	SendThreadManager::Instance().finalize();

	// 1. 타이머를 해제
	Core::TimerTask::Instance().stop();
}
