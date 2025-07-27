////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		Item 치트
///
///	@ date		2024-3-20
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Cheat.h"

class CheatItem
	:
	public Cheat
{
protected:

public:
	/// 생성자
	CheatItem();

	/// 핸들러
	virtual void onHandler(PlayerPtr player, const Core::StringList& params) override;

private:
	/// 인벤토리에 추가한다.
	void _add(PlayerPtr player, const Core::StringList& params) const;

	/// 인벤토리에서 삭제한다.
	void _del(PlayerPtr player, const Core::StringList& params) const;

	/// 인벤토리아이템을 사용한다.
	void _use(PlayerPtr player, const Core::StringList& params) const;

	/// 인벤토리아이템을 장착한다.
	void _equip(PlayerPtr player, const Core::StringList& params) const;

};
