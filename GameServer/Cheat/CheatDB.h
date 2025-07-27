////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		DB 치트
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "Cheat/Cheat.h"

class CheatDB
	:
	public Cheat
{
public:
	/// 생성자
	CheatDB();

	/// 핸들러
	virtual void onHandler(PlayerPtr player, const Core::StringList& params) override;

private:
	/// 리셋한다
	void _reset(PlayerPtr player, const Core::StringList& params);

	/// 정보를 출력한다
	void _info(PlayerPtr player, const Core::StringList& params);

};
