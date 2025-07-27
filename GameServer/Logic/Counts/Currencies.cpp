#include "Currencies.h"

#include <Core/Math/Numeric.h>
#include <Data/Info/InfoEnumsConv.h>
#include <Protocol/Struct/PktGrowth.h>
#include <Protocol/Struct/PktTypes.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
Currencies::Currencies()
{
	reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스탯을 설정한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void Currencies::set(ECurrencyType type, int32_t value)
{
	if ((size_t)(type) >= (size_t)ECurrencyType::Max)
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
int32_t Currencies::get(ECurrencyType type) const
{
	if ((size_t)(type) >= (size_t)(ECurrencyType::Max))
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
void Currencies::exportTo(PktCurrencies& dest) const
{
	for (size_t i = 0; i < (size_t)ECurrencyType::Max; ++i)
	{
		auto value = at(i);
		if (value)
		{
			PktCurrency pkt((ECurrencyType)(i), (int32_t)0, value);
			dest.emplace_back(std::move(pkt));
		}
	}
}

