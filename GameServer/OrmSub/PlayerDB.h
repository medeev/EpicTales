////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		OrmPlayer 상속 클래스
///
///	@ date		2024-3-7
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms/OrmPlayer.h>

class PlayerDB : public DBOrm::Player
{
public:
	/// 커스텀 캐시에 반영한다.
	virtual void applyCache(DBOrm::OrmObjectPtr origin) const override;

	/// 클론한다.
	std::shared_ptr<PlayerDB> clone() const;

	/// 위치를 반환한다.
	Vector getLocation() const;

	/// 방향을 반환한다.
	Vector getDir() const;
};

