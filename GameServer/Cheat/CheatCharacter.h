////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		Character 치트
///
///	@ date		2024-3-20
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Cheat.h"

class CheatCharacter
	:
	public Cheat
{
protected:

public:
	/// 생성자
	CheatCharacter();

	/// 핸들러
	virtual void onHandler(PlayerPtr player, const Core::StringList& params) override;

private:
	/// 몽당 획득한다.
	void _addall(PlayerPtr player, const Core::StringList& params) const;
};
