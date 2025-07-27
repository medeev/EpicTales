

#include "Pch.h"
#include "CheatManager.h"

#include "Actor/Player.h"
#include "Cheat.h"
#include "Config.h"

static CheatManager s_instanceCheatManager;		///< 인스턴스


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		명령어를 추가한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatManager::addCommand(const std::string& command, Cheat* cheat)
{
	std::string cmd = Core::StringUtil::ToLower(command);

	if (!_commands.insert({ cmd, cheat }).second)
		WARN_LOG("already exist command: %s", cmd.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		명령어를 수행한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatManager::processCommand(PlayerPtr player, const std::string& command)
{
	std::string manualCmd = Core::StringUtil::Trim(command.substr(1));
	Core::StringList params = Core::StringUtil::Split(manualCmd, " ");
	if (params.empty())
	{
		WARN_LOG("cheat parameter is empty");
		return;
	}

	std::string cmd = Core::StringUtil::ToLower(params[0]);
	params.erase(params.begin());

	auto it = _commands.find(cmd);
	if (it == _commands.end())
	{
		WARN_LOG("invalid cheat command: %s", cmd.c_str());
		return;
	}

	it->second->onHandler(player, params);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		채팅 명령어를 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CheatManager::handleChat(PlayerPtr player, const std::string& msg)
{
	if (!Config::GetCheatEnabled())
		return false;

	if (msg[0] != '/')
		return false;

	processCommand(player, msg);
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		인스턴스를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatManager& CheatManager::GetInstance()
{
	static CheatManager g_instance;
	return g_instance;
}
