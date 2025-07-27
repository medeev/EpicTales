////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		액터 이동에 쓰는 열거자
///
///	@ date		2024-3-25
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


enum class EPosTokenType : int32_t
{
	None,
	TokenNear0 = 1,
	TokenNear1 = 1 << 1,
	TokenNear2 = 1 << 2,
	TokenNear3 = 1 << 3,
	TokenNear4 = 1 << 4,
	TokenNear5 = 1 << 5,
	TokenNear6 = 1 << 6,
	TokenNear7 = 1 << 7,
	TokenNear8 = 1 << 8,
	TokenNear9 = 1 << 9,
	TokenNear10 = 1 << 10,
	TokenNear11 = 1 << 11,
	TokenFar0 = 1 << 12,
	TokenFar1 = 1 << 13,
	TokenFar2 = 1 << 14,
	TokenFar3 = 1 << 15,
	TokenFar4 = 1 << 16,
	TokenFar5 = 1 << 17,
	TokenFar6 = 1 << 18,
	TokenFar7 = 1 << 19,
	TokenFar8 = 1 << 20,
	TokenFar9 = 1 << 21,
	TokenFar10 = 1 << 22,
	TokenFar11 = 1 << 23,
	Max = -1
};
