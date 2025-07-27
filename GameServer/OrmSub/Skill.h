////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		OrmSkill 상속 클래스
///
///	@ date		2024-5-2
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms/OrmSkill.h>

class SkillInfo;
class PktSkill;
class Skill : public DBOrm::Skill
{
	Skill() = delete;
protected:
	const SkillInfo& _skillInfo;
public:
	/// 생성자
	Skill(const SkillInfo& skillInfo) : _skillInfo(skillInfo) {}

	/// 커스텀 캐시에 반영한다.
	virtual void applyCache(DBOrm::OrmObjectPtr origin) const override;

	const SkillInfo& getInfo() const {
		return _skillInfo;
	}

	std::shared_ptr<Skill> clone() const;
};

typedef std::shared_ptr<Skill> SkillPtr;