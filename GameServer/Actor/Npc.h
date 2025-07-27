////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		엔피시 클래스
///
///	@ date		2024-3-13
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Actor.h"
#include "Logic/Npc/NpcMoveComponent.h"
#include "Logic/NpcAi/NpcAiComponent.h"
#include "Logic/Skill/SkillComponent.h"
#include "Logic/WorldEntity/SpawnerTypes.h"

enum class NpcPositionType
{
	CombatStartPos,
};

class PktNpcInfo;
class NpcInfo;
class Npc
	:
	public Actor
{
	typedef Actor super;
	typedef std::unordered_map<NpcPositionType, Vector> LocationTypeMap;

private:
	NpcSpawnerWeakPtr _spawnerWeak; ///< 스포너
	const NpcInfo& _info; ///< 인포
	Core::Transform _spawnTransform; ///< 스폰된 포지션
	LocationTypeMap _locationMap; ///< 타입별 위치값들

	NpcAiComponent _npcAIComponent;
	NpcMoveComponent _npcMoveComponent;
	SkillComponent _skillComponent;
	int32_t _minSkillDist; ///< Npc가 사용한 스킬중에 가장 사거리가 짧은 스킬의 거리
	bool _isCombating;
public:
	/// 생성자
	Npc(const NpcInfo& info);

	/// 초기화 한다
	virtual void initialize() override;

	/// 정리 한다
	virtual void finalize() override;

	/// 정보를 내보낸다
	virtual void exportTo(PktSightEnterNotify& dest) const override;

	/// 정보를 내보낸다.
	virtual void exportTo(PktNpcInfo& dest) const;

	NpcMoveComponent& getNpcMoveComponent() {
		return _npcMoveComponent;
	}
	const NpcMoveComponent& getNpcMoveComponent() const {
		return _npcMoveComponent;
	}

	NpcAiComponent& getNpcAiComponent() {
		return _npcAIComponent;
	}
	const NpcAiComponent& getNpcAiComponent() const {
		return _npcAIComponent;
	}

	SkillComponent& getSkillComponent() {
		return _skillComponent;
	}
	const SkillComponent& getSkillComponent() const {
		return _skillComponent;
	}

	/// 스포너의 래퍼를 설정한다.
	void setSpawner(NpcSpawnerPtr spawner);

	/// 스폰너를 리턴한다.
	NpcSpawnerPtr getSpawner() {
		return _spawnerWeak.lock();
	}

	/// 스폰너를 리턴한다.
	const NpcSpawnerPtr getSpawner() const {
		return _spawnerWeak.lock();
	}

	/// 정보를 가져온다.
	const NpcInfo& getInfo() const {
		return _info;
	}
	PktInfoId getInfoId() const;

	/// 캡슐 반지름을 구한다.
	virtual float getRadius() const;

	/// 캡슐 높이를 구한다.
	virtual float getHalfHeight() const;

	/// 사용한 스킬중 가장 짧은거리
	int32_t getMinSkillDist() const {
		return _minSkillDist;
	}
	void setMinSkillDist(int32_t dist) {
		_minSkillDist = dist;
	}

	/// 월드에 진입한다.
	void enterWorld(World& world);

	/// 스폰된 포지션을 설정한다.
	void setSpawnTransform(const Core::Transform& transform) {
		_spawnTransform = transform;
	}

	/// 스폰된 포지션을 반환한다.
	const Core::Transform& getSpawnTransform() const {
		return _spawnTransform;
	}

	/// 루트포지션값을 반환한다.
	Vector getRootLocation() const override;

	/// 위치를 추가한다.
	void setLocationByType(NpcPositionType type, const Vector& location) {
		_locationMap[type] = location;
	}

	bool isCombat() const {
		return _isCombating;
	}

	/// 위치를 반환한다.
	bool getLocationByType(NpcPositionType type, Vector& outLocation);

	/// 데미지를 입는다.
	virtual void endPlayPost() override;
	/// 전투에 진입한다/나간다.
	void enterCombat();
	void exitCombat();
	
};
