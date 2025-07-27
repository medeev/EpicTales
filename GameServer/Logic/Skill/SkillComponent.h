////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		컴포넌트 클래스
///
///	@ date		2024-4-26
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <DB/CacheTx.h>
#include <Protocol/Struct/PktTypes.h>

#include "Actor/ActorTypes.h"
#include "Component/IComponent.h"
#include "Logic/Cooltime/CoolTimeManager.h"
#include "OrmSub/Skill.h"

class Item;
class Player;
class SkillComponent
	:
	public IComponent, 
	public CoolTimeManager
{
protected:
	Actor& _actor; ///< 액터

	std::map<PktInfoId, SkillPtr> _skills; /// 플레이어의 DB스킬
public:
	/// 생성자
	SkillComponent(Actor& actor);

	/// 소멸자
	virtual ~SkillComponent();

	/// 아이템을 추가한다.
	void insertSkill(SkillPtr skill);

	/// 아이템을 찾는다.
	const SkillPtr findSkill(PktInfoId infoId) const;
	SkillPtr findSkill(PktInfoId infoId);

	/// 아이템을 DB에 추가한다.
	void insertSkill(CacheTx& cacheTx, PktInfoId infoId) const;

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// DB객체로 초기화 한다
	virtual bool initializeDB(const DataBaseTarget&) override;

	/// 정리 한다
	virtual void finalize() override;

	/// DB객체로 초기화 한다
	bool _initByList(const std::list<std::shared_ptr<DBOrm::Skill>>& orms);

};
