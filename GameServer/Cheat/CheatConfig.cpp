

#include "Pch.h"
#include "CheatConfig.h"

#include "Config.h"

static CheatConfig s_instanceCheatConfig;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatConfig::CheatConfig()
	:
	Cheat("config")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatConfig::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.empty())
		return;

	if (params[0] == "debugShape")
	{
		_debugShape(player, params);
	}
	else if (params[0] == "debugMove")
	{
		_debugMove(player, params);
	}

}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스킬컬리젼을 보이게한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatConfig::_debugShape(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.size() < 2)
		return;

	auto value = Core::TypeConv::ToBool(params[1]);
	Config::SetDebugShape(value);
}

void CheatConfig::_debugMove(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.size() < 2)
		return;

	auto value = Core::TypeConv::ToBool(params[1]);
	Config::SetDebugNpcMove(value);
}
