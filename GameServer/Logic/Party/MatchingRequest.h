////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		매칭
///
///	@ date		2024-8-13
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Core/Util/Time.h>
#include <map>
#include <memory>

#include "Actor/ActorTypes.h"

class MatchingRequest;

/// 매칭 요청 포인터
typedef std::shared_ptr<MatchingRequest> MatchingRequestPtr;

/// 매칭 요청 포인터 목록
typedef std::list<MatchingRequestPtr> MatchingRequestPtrList;

/// 매칭 요청 포인터 맵
typedef std::map<int64_t, MatchingRequestPtr> MatchingRequestPtrByIdMap;


class MatchingRequest : 
	public std::enable_shared_from_this< MatchingRequest >
{
	/// 매칭 정보
	struct MatchingData
	{
		Core::Time _requestTime;
		uint32_t _filterId;
		bool _inProgress;
		bool _isInstantStart;

		MatchingData() : _inProgress(false), _isInstantStart(false) {}
		MatchingData(uint32_t filterId, bool isInstantStart) :
			_requestTime(Core::Time::GetCurTime()),
			_filterId(filterId),
			_inProgress(false),
			_isInstantStart(isInstantStart) {}
	};

	/// 매칭 데이터 맵
	typedef std::map< uint32_t, MatchingData > MatchingDataByFilterIdMap;

	MatchingDataByFilterIdMap _matchingDataByFilterIdMap; ///< 매칭 데이터 맵
	Core::Time _matchingReserveCompleteUpdateTime; ///< 매칭 예약 성공 여부 갱신 시간
	bool _isMatchingReserveComplete; ///< 매칭 예약 성공 여부
	PlayerWeakMap _playerWeaks; ///< 매칭에 참여한 플레이어목록

public:
	/// 생성자
	MatchingRequest();

	/// 설명을 반환한다.
	std::string toString() const;

	/// 매칭진행중인지 반환한다.
	bool isProgress(uint32_t filterId) const;

	/// 플레이어 숫자를 반환한다.
	int32_t getPlayerCount() const;

	/// 매칭예약완료시간을 설정한다.
	void setMatchingReserveCompleteTime(const Core::Time& t) {
		_matchingReserveCompleteUpdateTime = t;
	}
};
