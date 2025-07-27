////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		치트 관리자
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Core/Util/Singleton.h>
#include <unordered_map>

#include "Actor/ActorTypes.h"

class Cheat;

class CheatManager
{
private:
	std::unordered_map< std::string, Cheat* > _commands;	///< 목록

public:
	/// 명령어를 추가한다
	void addCommand(const std::string& command, Cheat* cheat);

	/// 명령어를 수행한다
	void processCommand(PlayerPtr player, const std::string& command);

	/// 채팅 명령어를 처리한다
	bool handleChat(PlayerPtr player, const std::string& msg);

public:
	/// 인스턴스를 반환한다
	static CheatManager& GetInstance();
};
