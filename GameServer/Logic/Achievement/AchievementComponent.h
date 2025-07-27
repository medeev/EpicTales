////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		컴포넌트 클래스
///
///	@ date		2024-10-11
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Data/Info/InfoEnums.h>
#include <Protocol/Struct/PktTypes.h>

#include "Component/IComponent.h"
#include "OrmSub/OrmSubType.h"

class PktAchievement;
class CacheTx;
class Player;
class AchievementComponent : public IComponent
{
public:
	
private:
	Player& _player; ///< Player
	
public:
	/// 생성자
	AchievementComponent(Player& player);

	/// 소멸자
	virtual ~AchievementComponent();

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// 정리 한다
	virtual void finalize() override;

public:
	/// 추가한다.
	bool insertAchievement(
		CacheTx& cacheTx,
		PktInfoId infoId
	) const;

	bool insertAchievement(AchievementPtr achievement);

	/// 모든업적을 내보낸다.
	void exportTo(std::vector<PktAchievement>& pktAchievements) const;

};
