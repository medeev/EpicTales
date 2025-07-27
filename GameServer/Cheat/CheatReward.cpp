

#include "Pch.h"
#include "CheatReward.h"

#include <Data/Info/NpcInfo.h>
#include <Data/Info/RewardInfo.h>
#include <Protocol/Struct/PktRoom.h>

#include "Logic/PlayerManager.h"
#include "Logic/Reward/RewardProcess.h"
#include "User/UserManager.h"
#include "DB/CacheTx.h"

static CheatReward s_instanceCheatReward;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatReward::CheatReward()
	:
	Cheat("reward")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatReward::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (!player)
		return;

	if (params.empty())
		return;

	if (params[0] == "drop")
	{
		_drop(player, params);
		return;
	}
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  아이템을 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatReward::_drop(PlayerPtr player, const Core::StringList& params) const
{
	if (!player)
		return;

	if (params.size() != 2)
		return;

	auto rewardStr = params[1];

	auto rewardGroupId = Core::TypeConv::ToInteger(rewardStr);

	RewardInfoByGroupIdPtr rewardGroup(rewardGroupId);
	if (!rewardGroup)
	{
		WARN_LOG("not found rewardGroup.  rewardGroupId: %d]", rewardGroupId);
		return;
	}

	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	RewardProcess::Process(*cacheTx, player, player, rewardGroup);
	cacheTx->run();
}
