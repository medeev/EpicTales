


#include "Pch.h"
#include "Matching.h"

#include <Core/Thread/ThreadChecker.h>
#include <Core/Util/IdFactory.h>

#include "PartyManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
Matching::Matching(WorldInfo& worldInfo)
	: 
	_matchingWorldInfo(worldInfo), 
	_delta(0), 
	_time(0), 
	_removable(false),
	_filterId(0),
	_targetTeamCount(1),
	_targetNumOfPlayerByTeam(5)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  업데이트 한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Matching::update()
{
	_updateMatching();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  플레이어 수를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
int32_t Matching::getPlayerCount() const
{
	return (int32_t)_players.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	파티에서 작업을 한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Matching::runTask(const Core::TaskCaller& taskCaller, const Core::TaskManager::Task& task)
{
	PartyManager::Instance().runTask(taskCaller, task);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 스레드가 올바른지 체크한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Matching::checkThread() const
{
	if (PartyManager::Instance().isRunning())
	{
		return Core::ThreadChecker::Check(
			PartyManager::Instance().getId(),
			"Party, id: %llu",
			_filterId);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  순회하면서 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Matching::foreachPlayer(std::function<void(PlayerPtr)> func)
{
	for (const auto& playerWeak : _players | std::views::values)
	{
		auto player = playerWeak.lock();
		if (!player)
			continue;

		if (func)
			func(player);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  매칭목록을 업데이트한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Matching::_updateMatching()
{
	static const int32_t matchingPlayerCount = 5;

	

	while (!_matchingRequests.empty())
	{
		auto request = _matchingRequests.front();
		_matchingRequests.pop_front();

		/// 매칭 가능한 파티 목록 찾기
		MatchingRequestPtrList completeList;
		if (_enterMatching(request, completeList))
		{
			for (const auto& matchingRequest : completeList)
				INFO_LOG("Match completed. [matchingRequest: %s]", matchingRequest->toString().c_str());
			break;
		}

		_matchingRequests.push_back(request);
	}

	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  매칭을 시도한다. 매칭성공시 참 반환
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Matching::_enterMatching(MatchingRequestPtr request, MatchingRequestPtrList& outList)
{
	ENSURE(request, return false);

	if (request->isProgress(_filterId))
		return false;

	auto targetPlayerCount = _targetTeamCount * _targetNumOfPlayerByTeam;
	ENSURE(targetPlayerCount > 0, return false);

	auto requestPlayerCount = request->getPlayerCount();

	bool makeSucc = false;
	int32_t totalCount = requestPlayerCount;
	for (const auto& request : _matchingRequests)
	{
		if (totalCount + request->getPlayerCount() == targetPlayerCount)
		{
			outList.push_back(request);
			makeSucc = true;
			break;
		}

		if (totalCount + request->getPlayerCount() > targetPlayerCount)
			continue;

		totalCount += request->getPlayerCount();
		outList.push_back(request);
	}

	for (auto& request : outList)
		request->setMatchingReserveCompleteTime(Core::Time::GetCurTime());

	return makeSucc;
}
