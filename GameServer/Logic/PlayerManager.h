////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		플레이어 관리자
///
///	@ date		2024-3-7
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Core/ThreadSafeContainer/TSMap.h>
#include <Protocol/Struct/PktTypes.h>

#include "Actor/ActorTypes.h"

class PlayerManager
	:
	public Core::Singleton<PlayerManager>,
	public Core::TSMap<PktObjId, PlayerPtr>
{
	friend class Core::Singleton<PlayerManager>;
protected:
};

class PlayerCloseManager
	:
	public Core::Singleton<PlayerCloseManager>,
	public Core::TSMap<PktObjId, PlayerWeakPtr> /// Key값 CloseCompoent kCloseClearCheckSec 대기중인 끊긴유저의 ID, Value:플레이어식별자
{
	friend class Core::Singleton<PlayerCloseManager>;
protected:
};
