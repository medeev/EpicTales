////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		Config 치트
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Cheat.h"

class CheatConfig
	:
	public Cheat
{
protected:

public:
	/// 생성자
	CheatConfig();

	/// 핸들러
	virtual void onHandler(PlayerPtr player, const Core::StringList& params) override;

private:
	void _debugShape(PlayerPtr player, const Core::StringList& params);
	void _debugMove(PlayerPtr player, const Core::StringList& params);
};
