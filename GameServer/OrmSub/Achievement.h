////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		OrmAchievement 상속 클래스
///
///	@ date		2024-10-17
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms/OrmAchievement.h>

class AchievementInfo;
class PktAchievement;
class Achievement : public DBOrm::Achievement
{
	Achievement() = delete;
protected:
	const AchievementInfo& _achievementInfo;
public:
	/// 생성자
	Achievement(const AchievementInfo& achievementInfo) : _achievementInfo(achievementInfo) {}

	/// 커스텀 캐시에 반영한다.
	virtual void applyCache(DBOrm::OrmObjectPtr origin) const override;

	const AchievementInfo& getInfo() const {
		return _achievementInfo;
	}
	void exportTo(PktAchievement& dest) const;

	std::shared_ptr<Achievement> clone() const;
};

