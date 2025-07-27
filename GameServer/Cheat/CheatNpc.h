////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		Npc 치트
///
///	@ date		2024-3-26
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Cheat.h"

class CheatNpc
	:
	public Cheat
{
protected:

public:
	/// 생성자
	CheatNpc();

	/// 핸들러
	virtual void onHandler(PlayerPtr player, const Core::StringList& params) override;

private:
	/// Npc를 생성합니다.
	void _create(PlayerPtr player, const Core::StringList& params);

	/// Npc를 죽인다.
	void _killByRange(PlayerPtr player, const Core::StringList& params);

	/// Npc를 죽인다.
	void _kill(PlayerPtr player, const Core::StringList& params);

	/// Npc에게 효과를 부여한다.
	void _apply(PlayerPtr player, const Core::StringList& params);

	/// 정보를 출력합니다.
	void _infos(PlayerPtr player, const Core::StringList& params);

	void _invisible(NpcPtr npc, bool apply) const;
	void _invincible(NpcPtr npc, bool apply) const;
	void _unmoveable(NpcPtr npc, bool apply) const;
};
