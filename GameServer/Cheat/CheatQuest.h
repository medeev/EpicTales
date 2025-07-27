////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Quest 치트
///
///	@ date		2024-3-20
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Cheat.h"

class CheatQuest
	:
	public Cheat
{
protected:

public:
	/// 생성자
	CheatQuest();

	/// 핸들러
	virtual void onHandler(PlayerPtr player, const Core::StringList& params) override;

private:
	
	/// 퀘스트를 완료한다.
	void _start(PlayerPtr player, const Core::StringList& params) const;

	/// 퀘스트를 완료가능상태로 변경한다.
	void _comp(PlayerPtr player, const Core::StringList& params) const;

	/// 완료 퀘스트를 추가한다.
	void _add(PlayerPtr player, const Core::StringList& params) const;

	/// 완료퀘스트를 삭제한다.
	void _del(PlayerPtr player, const Core::StringList& params) const;

};
