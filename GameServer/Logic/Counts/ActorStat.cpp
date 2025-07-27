#include "ActorStat.h"

#include <Core/Math/Numeric.h>
#include <Data/Info/CharacterInfo.h>
#include <Data/Info/InfoEnumsConv.h>
#include <Data/Info/ItemInfo.h>
#include <Data/Info/NpcInfo.h>
#include <Protocol/Struct/PktTypes.h>
#include <Protocol/Struct/PktActor.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
ActorStat::ActorStat()
{
	reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  캐릭터 스탯으로 액터 스탯 초기화
////////////////////////////////////////////////////////////////////////////////////////////////////
ActorStat::ActorStat(const CharacterInfo& info)
{
	fill(Core::Numeric::MaxInt32);

	set(EActorStatType::STR, info.getSTR());
	set(EActorStatType::DEX, info.getDEX());
	set(EActorStatType::INT, info.getINT());

	/// 정보에 모든 데이터가 set되어있는지 체크한다.
	for (int32_t i = 0; i < size(); ++i)
	{
		EActorStatType statType = (EActorStatType)i;
		if (at(i) == Core::Numeric::MaxInt32)
		{
			set(statType, 0);
			WARN_LOG("%s character stat is not set", convert(statType).c_str());
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  Npc 스탯으로 액터 스탯 초기화
////////////////////////////////////////////////////////////////////////////////////////////////////
ActorStat::ActorStat(const NpcInfo& info)
{
	fill(Core::Numeric::MaxInt32);

	set(EActorStatType::STR, 0);
	set(EActorStatType::DEX, 0);
	set(EActorStatType::INT, 0);

	/// 정보에 모든 데이터가 set되어있는지 체크한다.
	for (int32_t i = 0; i < size(); ++i)
	{
		EActorStatType statType = (EActorStatType)i;
		if (at(i) == Core::Numeric::MaxInt32)
		{
			set(statType, 0);
			WARN_LOG("%s npc stat is not set", convert(statType).c_str());
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  ItemInfo스탯으로 초기화
////////////////////////////////////////////////////////////////////////////////////////////////////
ActorStat::ActorStat(const ItemInfo& info)
{
	fill(Core::Numeric::MaxInt32);

	set(EActorStatType::STR, info.getSTR());
	set(EActorStatType::DEX, info.getDEX());
	set(EActorStatType::INT, info.getINT());

	/// 정보에 모든 데이터가 set되어있는지 체크한다.
	for (int32_t i = 0; i < size(); ++i)
	{
		EActorStatType statType = (EActorStatType)i;
		if (at(i) == Core::Numeric::MaxInt32)
		{
			set(statType, 0);
			WARN_LOG("%s npc stat is not set", convert(statType).c_str());
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스탯을 설정한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void ActorStat::set(EActorStatType type, int32_t value)
{
	if ((size_t)(type) >= (size_t)EActorStatType::Max)
	{
		WARN_LOG("invalid type, %d", (int32_t)(type));
		return;
	}

	if ((size_t)(type) >= size() || (size_t)type < 0)
	{
		WARN_LOG("invalid type size over, %d", (int32_t)(type));
		return;
	}

	at((size_t)(type)) = value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스탯을 가져온다
////////////////////////////////////////////////////////////////////////////////////////////////////
int32_t ActorStat::get(EActorStatType type) const
{
	if ((size_t)(type) >= (size_t)(EActorStatType::Max))
	{
		WARN_LOG("invalid type, %d", (int32_t)(type));
		return 0;
	}
	if ((size_t)(type) >= size() || (size_t)type < 0)
	{
		WARN_LOG("invalid type size over, %d", (int32_t)(type));
		return 0;
	}

	return at((size_t)(type));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	정보를 내보낸다
////////////////////////////////////////////////////////////////////////////////////////////////////
void ActorStat::exportTo(PktActorStats& dest) const
{
	for (size_t i = 0; i < (size_t)EActorStatType::Max; ++i)
	{
		auto value = at(i);
		if (value)
		{
			PktActorStat pkt((EActorStatType)(i), (int32_t)0, value);
			dest.emplace_back(std::move(pkt));
		}
	}
}
