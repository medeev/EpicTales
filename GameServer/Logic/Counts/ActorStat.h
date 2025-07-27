////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		액터스탯 클래스
///
///	@ date		2024-3-13
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Data/Info/InfoEnums.h>
#include <Protocol/Struct/PktTypes.h>

class CharacterInfo;
class NpcInfo;
class ItemInfo;
class GrowthInfo;
class StatPointInfo;
class ActorStat
	:
	public std::array< int32_t, (size_t)EActorStatType::Max >
{
public:
	ActorStat();

	/// 캐릭터 스탯으로 액터스탯 초기화
	ActorStat(const CharacterInfo& info);

	/// Npc 스탯으로 액터스탯 초기화
	ActorStat(const NpcInfo& info);

	/// ItemInfo 스탯으로 액터스탯 초기화
	ActorStat(const ItemInfo& info);

	/// 스탯을 리셋한다.
	void reset() { 
		std::fill(begin(), end(), 0); }

	/// 스탯을 설정한다.
	void set(EActorStatType type, int32_t value);

	/// 스탯을 가져온다.
	int32_t get(EActorStatType type) const;

	/// operator+= 정의
	ActorStat& operator+=(const ActorStat& other) {
		for (size_t i = 0; i < (size_t)EActorStatType::Max; ++i) {
			(*this)[i] += other[i];
		}
		return *this;
	}

	/// operator+= 정의
	ActorStat& operator-=(const ActorStat& other) {
		for (size_t i = 0; i < (size_t)EActorStatType::Max; ++i) {
			(*this)[i] -= other[i];
		}
		return *this;
	}

	/// operator + 정의
	ActorStat operator+(const ActorStat& other) const {
		ActorStat result = *this;
		result += other;
		return result;
	}

	/// operator - 정의
	ActorStat operator-(const ActorStat& other) const {
		ActorStat result = *this;
		result -= other;
		return result;
	}

	/// 정보를 내보낸다
	void exportTo(PktActorStats& dest) const;


protected:
};
