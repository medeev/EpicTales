////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		Npc스폰 담당 클래스
///
///	@ date		2024-3-15
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <memory>
#include <Protocol/Struct/PktTypes.h>

class NpcSpawnData;
class NpcInfo;
class World;
class Npc;
class NpcSpawner
	:
	public std::enable_shared_from_this< NpcSpawner >
{
	/// Npc 식별자 목록 타입 정의
	typedef std::map<PktObjId, NpcPtr> Npcs;

	/// Npc 스폰 시간 정의
	typedef std::deque<int64_t> SpawnTimes;

	/// NPC 스폰 타입
	enum class ESpawnType
	{
		Default,          ///< 기본
		SpecificTime,     ///< 특정 시간
		SpecificTimeLoop, ///< 특정 시간부터 주기적으로
	};

private:
	World& _world; ///< 월드
	const NpcSpawnData& _npcSpawnInfo; ///< Npc스폰정보
	const NpcInfo& _npcInfo;      ///< Npc정보
	Npcs        _spawnedNpcs;  ///< 스폰한 NPC 목록
	SpawnTimes _spawnTimes;   ///< 스폰타임큐
	ESpawnType _spawnType;    ///< 스폰 타입
	uint32     _spawnedCount; ///< 스폰된 숫자
	std::list<Core::Transform> _spawnTransformPool; ///< 스폰 위치풀

public:
	/// 생성자
	NpcSpawner(World& world, const NpcSpawnData& info, const NpcInfo& npcInfo);

	/// 소멸자
	~NpcSpawner();

	/// 초기화한다.
	void init();

	/// 갱신 한다
	void update(int64_t curTime);

	/// 대상을 스폰타이머에서 삭제한다.
	void removeSpawnTimer(const Npc& npc);

	/// 스폰되어 있는 Npc 식별자 목록을 반환한다
	const Npcs& getNpcList() const {
		return _spawnedNpcs;
	}

	/// 스폰타이머를 추가한다.
	void addSpawnTimer(int64_t timeValue);

	/// 스폰타이머를 클리어 한다.
	void clearSpawnTimer() {
		_spawnTimes.clear();
	}
protected:
	/// 스폰 시킨다.
	bool _doSpawn();
};
