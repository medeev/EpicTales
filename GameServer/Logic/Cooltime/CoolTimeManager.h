
#pragma once

#include <map>

constexpr int64_t COOLTIME_STACK_MAX_TIME = 24 * 60 * 60 * 1000 * -1;
typedef int8_t SlotIdx;

class PktCoolTime;
class Actor;

enum class ECoolTimeState
{
	Set,
	Pending
};

class CoolTime
{
public:
	CoolTime()
		: _endCoolTime(0), _state(ECoolTimeState::Set)
	{
	}
	int64_t _endCoolTime; ///< 쿨타임 종료시간
	ECoolTimeState _state; ///< 쿨타임의 설정 상태
};

class CoolTimeManager
{
protected:
	/// 스킬 쿨타임 맵 타입 정의
	typedef std::map< uint32_t, CoolTime > CoolTimeMap;

	/// 스킬 슬롯 쿨타임 맵 타입 정의
	typedef std::map< SlotIdx, CoolTime > SlotCoolTimeMap; // 슬롯인덱스/쿨타임

protected:
	CoolTimeMap     _coolTimes;     ///< 쿨타임 맵
	SlotCoolTimeMap _slotCoolTimes; ///< 슬롯 쿨타임 맵

public:
	/// 쿨타임을 리셋한다.
	void resetCoolTime();

	/// 스킬 쿨타임을 체크하여 가능여부를 반환한다.
	bool checkCoolTime(
		uint32_t key, 
		SlotIdx SlotIdx = -1, 
		int64_t curTime = Core::Time::GetCurTimeMSec()) const;

	/// 쿨타임이 등록되어있는지 여부를 반환한다.
	bool isExist(uint32_t key) const;

	void setCoolTimeState(
		uint32_t key, 
		SlotIdx slotIdx = -1, 
		ECoolTimeState state = ECoolTimeState::Set);

	/// 스킬 쿨타임을 설정한다.
	void setCoolTime(
		int64_t curTime, 
		uint32_t key, 
		SlotIdx slotIdx = -1, 
		int64_t coolTime = 0);

	/// 스킬 쿨타임을 증가 시킨다.
	void increaseCoolTime(int64_t coolTime);

	/// 특정 키의 스킬 쿨타임을 증가시킨다.
	void increaseCoolTime(uint32_t key, int64_t coolTime);

	/// 스킬 쿨타임을 감소 시킨다.
	void decreaseCoolTime(int64_t coolTime);

	/// 특정 키의 스킬 쿨타임을 감소시킨다.
	void decreaseCoolTime(uint32_t key, int64_t coolTime);

	/// 스킬 쿨타임을 반환한다.
	bool getCoolTime(uint32_t key, CoolTime& outCooltime) const;

	/// 쿨타임정보 전체를 익스포트한다.
	//void exportTo(std::list<PktCoolTime>& pktCoolTimes) const;

	/// 쿨타임정보 대상을 익스포트한다.
	//void exportTo(uint32_t targetKey, std::list<PktCoolTime>& pktCoolTimes) const;
};