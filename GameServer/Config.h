////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		설정 클래스
///
///	@ date		2024-3-7
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Core/Config.h>

class Config
{
private:
	static Config ms_instance; ///< 인스턴스
	
public:
	/// 패킷 로그 활성화 여부
	CONFIG_MEMBER(bool, PacketDBLog, "PKT_DB_LOG", true);
	CONFIG_MEMBER(uint8_t, DBThdCount, "DB_THREAD_COUNT", 4);
	CONFIG_MEMBER(bool, CheatEnabled, "CHEAT_ENABLE", true);
	CONFIG_MEMBER(int32_t, SendThreadCount, "SEND_THREAD_COUNT", 2);
	CONFIG_MEMBER(int32_t, IOCPThreadCount, "IOCP_THREAD_COUNT", 20);
	CONFIG_MEMBER(bool, PingKickOut, "PING_KICK_OUT", true);


	CONFIG_MEMBER(std::string, LoginDbHost, "LOGIN_DB_HOST", std::string("127.0.0.1"));
	CONFIG_MEMBER(int16_t, LoginDbPort, "LOGIN_DB_PORT", 3306);
	CONFIG_MEMBER(std::string, LoginDbUser, "LOGIN_DB_USER", std::string("root"));
	CONFIG_MEMBER(std::string, LoginDbPass, "LOGIN_DB_PASS", std::string("dosdb!1"));
	CONFIG_MEMBER(std::string, LoginDbName, "LOGIN_DB_NAME", std::string("ny_login"));

	CONFIG_MEMBER(std::string, GameDbHost, "GAME_DB_HOST", std::string("127.0.0.1"));
	CONFIG_MEMBER(int16_t, GameDbPort, "GAME_DB_PORT", 3306);
	CONFIG_MEMBER(std::string, GameDbUser, "GAME_DB_USER", std::string("root"));
	CONFIG_MEMBER(std::string, GameDbPass, "GAME_DB_PASS", std::string("dosdb!1"));
	CONFIG_MEMBER(std::string, GameDbName, "GAME_DB_NAME", std::string("ny_game"));

	CONFIG_MEMBER(std::string, RedisHost, "REDIS_HOST", std::string("127.0.0.1"));

	CONFIG_MEMBER(bool, DebugShape, "DebugShape", false);
	CONFIG_MEMBER(bool, DebugNpcMove, "DebugNpcMove", false);
	CONFIG_MEMBER(bool, CollectPktBot, "CollectPktBot", false);

public:
};

