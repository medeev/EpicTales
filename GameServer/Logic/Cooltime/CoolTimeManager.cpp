
#include "Pch.h"
#include "CoolTimeManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	쿨타임을 리셋한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CoolTimeManager::resetCoolTime()
{
	_coolTimes.clear();
	_slotCoolTimes.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스킬 쿨타임을 체크하여 가능여부를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CoolTimeManager::checkCoolTime(uint32_t key, SlotIdx slotIdx, int64_t curTime) const
{
	const auto iter = _coolTimes.find(key);
	if (iter != _coolTimes.end())
	{
		const auto& coolTime = iter->second;
		if (coolTime._state == ECoolTimeState::Pending)
			return false;

		if (curTime < coolTime._endCoolTime)
			return false;
	}

	if (slotIdx >= 0)
	{
		const auto iterSlot = _slotCoolTimes.find(slotIdx);
		if (iterSlot != _slotCoolTimes.end())
		{
			const auto& coolTime = iter->second;
			if (coolTime._state == ECoolTimeState::Pending)
				return false;

			if (curTime < coolTime._endCoolTime)
				return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	쿨타임상태를 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CoolTimeManager::setCoolTimeState(uint32_t key, SlotIdx slotIdx, ECoolTimeState state)
{
	if (slotIdx >= 0)
		_slotCoolTimes[slotIdx]._state = state;

	_coolTimes[key]._state = state;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스킬 쿨타임을 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CoolTimeManager::setCoolTime(
	int64_t curTime,
	uint32_t key,
	SlotIdx slotIndex,
	int64_t coolTime)
{
	if (coolTime == 0)
		return;

	int64_t calcCoolTime = curTime + coolTime;

	if (slotIndex >= 0)
	{
		auto& coolTime = _slotCoolTimes[slotIndex];
		coolTime._state = ECoolTimeState::Set;
		coolTime._endCoolTime = calcCoolTime;
	}

	{
		auto& coolTime = _coolTimes[key];
		coolTime._state = ECoolTimeState::Set;
		coolTime._endCoolTime = calcCoolTime;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스킬 쿨타임을 증가 시킨다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CoolTimeManager::increaseCoolTime(int64_t msec)
{
	for (auto& coolTimePair : _coolTimes)
		coolTimePair.second._endCoolTime += msec;

	for (auto& coolTimePair : _slotCoolTimes)
		coolTimePair.second._endCoolTime += msec;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	특정키의 쿨타임을 증가시킨다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CoolTimeManager::increaseCoolTime(uint32_t key, int64_t coolTime)
{
	if (!key)
		return;

	{
		auto iter = _coolTimes.find(key);
		if (iter != _coolTimes.end())
			iter->second._endCoolTime += coolTime;
	}

	{
		auto iter = _slotCoolTimes.find(key);
		if (iter != _slotCoolTimes.end())
			iter->second._endCoolTime += coolTime;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	쿨타임을 감소 시킨다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CoolTimeManager::decreaseCoolTime(int64_t msec)
{
	for (auto& coolTimePair : _coolTimes)
		coolTimePair.second._endCoolTime -= msec;

	for (auto& coolTimePair : _slotCoolTimes)
		coolTimePair.second._endCoolTime -= msec;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	특정 키의 스킬 쿨타임을 감소시킨다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CoolTimeManager::decreaseCoolTime(uint32_t key, int64_t coolTime)
{
	{
		auto iter = _coolTimes.find(key);
		if (iter != _coolTimes.end())
			iter->second._endCoolTime -= coolTime;
	}

	{
		auto iter = _slotCoolTimes.find(key);
		if (iter != _slotCoolTimes.end())
			iter->second._endCoolTime -= coolTime;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	쿨타임을 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CoolTimeManager::getCoolTime(uint32_t key, CoolTime& outCooltime) const
{
	auto iter = _coolTimes.find(key);
	if (iter != _coolTimes.end())
	{
		outCooltime = iter->second;
		return true;
	}

	return false;
}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
///// @ brief	쿨타임정보를 익스포트한다.
//////////////////////////////////////////////////////////////////////////////////////////////////////
//void CoolTimeManager::exportTo(std::list<PktCoolTime>& pktCoolTimes) const
//{
//	auto curTime = Core::Time::GetCurTimeMSec();
//
//	for (const auto& pairVal : _coolTimes)
//	{
//		auto key = pairVal.first;
//		const auto& coolTime = pairVal.second;
//
//		pktCoolTimes.emplace_back(key, coolTime._endCoolTime);
//	}
//}
//
//void CoolTimeManager::exportTo(uint32_t targetKey, std::list<PktCoolTime>& pktCoolTimes) const
//{
//	auto curTime = Core::Time::GetCurTimeMSec();
//
//	for (const auto& pairVal : _coolTimes)
//	{
//		auto key = pairVal.first;
//		const auto& coolTime = pairVal.second;
//
//		if (key != targetKey)
//			continue;
//
//		pktCoolTimes.emplace_back(key, coolTime._endCoolTime);
//	}
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	쿨타임이 등록되어있는지 여부를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CoolTimeManager::isExist(uint32_t key) const
{
	auto iter = _coolTimes.find(key);
	if (iter == _coolTimes.end())
		return false;

	return true;
}
