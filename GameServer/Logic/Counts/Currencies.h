////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		재화수량 클래스
///
///	@ date		2024-3-13
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Data/Info/InfoEnums.h>
#include <Protocol/Struct/PktTypes.h>

class Currencies
	:
	public std::array< int32_t, (size_t)ECurrencyType::Max >
{
public:
	Currencies();

	/// 스탯을 리셋한다.
	void reset() { 
		std::fill(begin(), end(), 0); }

	/// 스탯을 설정한다.
	void set(ECurrencyType type, int32_t value);

	/// 스탯을 가져온다.
	int32_t get(ECurrencyType type) const;

	/// operator+= 정의
	Currencies& operator+=(const Currencies& other) {
		for (size_t i = 0; i < (size_t)ECurrencyType::Max; ++i) {
			(*this)[i] += other[i];
		}
		return *this;
	}

	/// operator+= 정의
	Currencies& operator-=(const Currencies& other) {
		for (size_t i = 0; i < (size_t)ECurrencyType::Max; ++i) {
			(*this)[i] -= other[i];
		}
		return *this;
	}

	/// 정보를 내보낸다
	void exportTo(PktCurrencies& dest) const;


protected:
};
