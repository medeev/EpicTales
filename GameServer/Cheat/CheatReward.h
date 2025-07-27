////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		Reward 치트
///
///	@ date		2024-3-20
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Cheat.h"

class CheatReward
	:
	public Cheat
{
protected:

public:
	/// 생성자
	CheatReward();

	/// 핸들러
	virtual void onHandler(PlayerPtr player, const Core::StringList& params) override;

private:
	/// 리워드그룹을 바닥에 생성한다.
	void _drop(PlayerPtr player, const Core::StringList& params) const;

};
