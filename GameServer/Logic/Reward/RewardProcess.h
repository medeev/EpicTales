////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		리워드보상
///
///	@date		2024-8-22
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Data/Info/RewardInfo.h>

#include "Actor/ActorTypes.h"

class CacheTx;
class EffectApply;
class RewardProcess
{
public:
	static void Process(
		CacheTx& cacheTx,
		ActorPtr self, 
		PlayerPtr receiver, 
		RewardInfoByGroupIdPtr rewardGroup);
};
