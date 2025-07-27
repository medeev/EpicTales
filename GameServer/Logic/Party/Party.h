////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		옥션
///
///	@ date		2024-8-13
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Core/Task/TaskCaller.h>
#include <Core/Task/TaskManager.h>

#include "Actor/ActorTypes.h"

class PartyManager;
class Party
{
	PartyManager& _partyManager; ///< 스레드
	uint64_t _id;                ///< 옥션 식별자
	int32_t _delta;              ///< 델타시간값
	int64_t _time;               ///< 시간
	PlayerWeakMap _players;     ///< 플레이어 목록
	bool _removable;             ///< 삭제여부
	Party() = delete;
public:
	/// 생성자
	Party(PartyManager&);

	/// 옥션 식별자를 반환한다.
	uint64_t getId() const {
		return _id;
	}

	/// 델타타임을 설정한다.
	void setDeltaMSec(int32_t delta) {
		_delta = delta;
	}

	/// 시간을 설정한다.
	void setTime(int64_t msec) {
		_time = msec;
	}

	/// 업데이트 한다.
	void update();

	/// 삭제설정을 한다.
	void setRemoveable(bool value) {
		_removable = value;
	}

	/// 삭제해야하는 여부를 반환한다.
	bool isRemoveable() const {
		return _removable;
	}

	/// 플레이어 수를 반환한다.
	int32_t getPlayerCount() const;

	/// 작업을 수행한다.
	void runTask(const Core::TaskCaller& taskCaller, const Core::TaskManager::Task& task);

	/// 스레드를 체크한다.
	bool checkThread() const;

	/// 순회하면서 처리한다
	void foreachPlayer(std::function<void(PlayerPtr)> func);
};
