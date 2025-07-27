////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		World 치트
///
///	@ date		2024-3-20
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Cheat.h"

class CheatWorld
	:
	public Cheat
{
protected:

public:
	/// 생성자
	CheatWorld();

	/// 핸들러
	virtual void onHandler(PlayerPtr player, const Core::StringList& params) override;

private:
	/// 정보를 출력한다.
	void _printInfo(PlayerPtr player, const Core::StringList& params) const;

	/// 정보를 출력한다.
	void _location(PlayerPtr player, const Core::StringList& params) const;

	/// 정보를 출력한다.
	void _project(PlayerPtr player, const Core::StringList& params) const;

	/// Cell정보를 출력한다.
	void _cell(PlayerPtr player, const Core::StringList& params) const;

	/// 특정위치에서 플레이어로의 Path 를 출력해본다.
	void _path(PlayerPtr player, const Core::StringList& params) const;

	/// 월드이동
	void _moveTo(PlayerPtr player, const Core::StringList& params);

	/// 월드상의 랜덤한 위치 찍기
	void _randomLocation(PlayerPtr player, const Core::StringList& params);
};
