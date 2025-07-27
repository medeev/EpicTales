////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		Player 치트
///
///	@ date		2024-3-26
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Cheat.h"

class CheatPlayer
	:
	public Cheat
{
protected:

public:
	/// 생성자
	CheatPlayer();

	/// 핸들러
	virtual void onHandler(PlayerPtr player, const Core::StringList& params) override;

private:
	/// 디비깅
	void _debug(PlayerPtr player, const Core::StringList& params) const;
	
	/// 플레이어 정보를 출력해본다.
	void _printInfo(PlayerPtr player, const Core::StringList& params) const;

	/// 경험치를 추가한다.
	void _exp(PlayerPtr player, const Core::StringList& params) const; 

	/// 남은 위치토큰을 본다.
	void _token(PlayerPtr player, const Core::StringList& params) const;
};
