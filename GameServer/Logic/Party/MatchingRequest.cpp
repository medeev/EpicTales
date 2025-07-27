


#include "Pch.h"
#include "MatchingRequest.h"

#include <Core/Thread/ThreadChecker.h>
#include <Core/Util/IdFactory.h>

MatchingRequest::MatchingRequest()
{

}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  설명을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string MatchingRequest::toString() const
{
	Core::StringList strList;
	for (const auto& matchingData : _matchingDataByFilterIdMap | std::views::values)
	{
		strList.emplace_back(
			STRINGUTIL_FORMAT(
				"[requestTime: %s, filterId: %u, inProgress: %d, isInstantStart: %d]",
				matchingData._requestTime.toString().c_str(),
				matchingData._filterId,
				matchingData._inProgress,
				matchingData._isInstantStart));
	}

	return STRINGUTIL_FORMAT(
		"[matchingDataByFilterIdMap: %s, matchingReserveCompleteUpdateTime: %s]",
		Core::StringUtil::Merge(strList, ", ").c_str(),
		_matchingReserveCompleteUpdateTime.toString().c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  매칭진행여부를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool MatchingRequest::isProgress(uint32_t filterId) const
{
	if (filterId == 0)
		for (auto& matchingData : _matchingDataByFilterIdMap | std::views::values)
			if (matchingData._inProgress)
				return true;

	auto it = _matchingDataByFilterIdMap.find(filterId);
	if (it == _matchingDataByFilterIdMap.end())
		return false;

	return it->second._inProgress;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  매칭요청에 포함된 플레이어 수를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
int32_t MatchingRequest::getPlayerCount() const
{
	return (int32_t)_playerWeaks.size();
}
