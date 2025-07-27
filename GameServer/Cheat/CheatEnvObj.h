////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		EnvObj 치트
///
///	@ date		2024-4-26
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Cheat.h"

class CheatEnvObj
	:
	public Cheat
{
public:
	/// 생성자
	CheatEnvObj();

	/// 핸들러
	virtual void onHandler( PlayerPtr player, const Core::StringList& params ) override;

private:
	/// 반경 안에 있는 가젯 하나를 컨트롤 한다.
	void _ctrlEnvObj( PlayerPtr player, const Core::StringList& params );

	/// 가젯 상태를 변경한다.
	void _changeState( PlayerPtr player, const Core::StringList& params );

	/// 디버깅 정보를 잠시 출력한다.
	void _showDebugInfo( PlayerPtr player, const Core::StringList& params );
};
