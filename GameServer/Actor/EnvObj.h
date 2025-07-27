////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		환경오브젝트
///
///	@ date		2024-3-2
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Data/Info/EnvObjInfo.h>

#include "Actor.h"
#include "ActorTypes.h"
#include "Logic/WorldEntity/SpawnerTypes.h"

class PktEnvObjState;
class EnvObjSpawnData;
class EnvObj
	:
	public Actor
{
	typedef Actor super;
	typedef std::map< PktObjId, ActorWeakPtr > ControlsMap;
protected:
	const EnvObjInfo& _info;		      ///< 인포
	const EnvObjSpawnData* _spawnInfo;     ///< 스폰정보
	EnvObjSpawnerWeakPtr _spawnerWeak;     ///< 스포너
	int64_t              _lifeTime;        ///< 유지시간
	int64_t              _nextChargeTime;  ///< 다음 충전시간
	uint32_t             _remainCtrlCount; ///< 현재 남은 횟수
	EEnvObjStateType     _envObjState;     ///< 상태
	ControlsMap          _controls;        ///< 컨트롤중인 액터목록
	Core::Transform _spawnTransform; ///< 스폰위치

public:
	/// 생성자
	EnvObj(const EnvObjInfo& info);

	/// 초기화 한다
	virtual void initialize() override;

	/// 정보를 내보낸다
	virtual void exportTo(PktSightEnterNotify& dest) const override;

	virtual float getRadius() const override {
		return 30;
	}
	virtual float getHalfHeight() const override {
		return 30;
	}

	/// 테이블 식별자를 반환한다.
	uint32_t getInfoId() const { return _info.getId(); }

	/// 정보를 내보낸다.
	void exportTo(PktEnvObjState& dest) const;

	/// 갱신한다
	virtual void update(int64_t curTime) override;

	/// 정보를 내보낸다.
	void exportTo(PktEnvObjInfo& dest) const;

	/// 스포너의 래퍼를 설정한다.
	void setSpawner(EnvObjSpawnerPtr spawner);

	/// 스폰너를 리턴한다.
	EnvObjSpawnerPtr getSpawner() {
		return _spawnerWeak.lock();
	}

	/// 스폰Transform을 반환한다
	const Core::Transform& getSpawnTransform() const {
		return _spawnTransform;
	}

	/// 스폰된 포지션을 설정한다.
	void setSpawnLocation(const Core::Transform& transform) {
		_spawnTransform = transform;
	}

	/// 스폰너를 리턴한다.
	const EnvObjSpawnerPtr getSpawner() const {
		return _spawnerWeak.lock();
	}

	/// 스폰정보를 가져온다.
	const EnvObjSpawnData* getSpawnInfo() const {
		return _spawnInfo;
	}

	/// 가젯정보를 반환한다.
	const EnvObjInfo& getInfo() const {
		return _info;
	}

	/// LifeTime 가져온다.
	int64_t getLifeTime() const {
		return _lifeTime;
	}

	/// LifeTime 설정한다.
	void setLifeTime(int64_t value) {
		_lifeTime = value;
	}

	/// 활성화 상태를 설정한다.
	EEnvObjStateType getEnvObjState() const {
		return _envObjState;
	}

	/// 활성화 상태를 설정한다.
	void setEnvObjState(EEnvObjStateType value) {
		_envObjState = value;
	}

	/// 남은 컨트롤 횟수를 반환한다.
	uint32_t getRemainCtrlCount() const {
		return _remainCtrlCount;
	}

	/// 남은 컨트롤 횟수를 설정한다.
	void setRemainCtrlCount(uint32_t count);

	/// 가젯 컨트롤 카운트가 최고치에 도달했는지 여부
	bool isFullControlCount() const;

	/// 가젯이 컨트롤 횟수 소진후 제거되는지 여부를 반환한다
	bool isRemainEmtpyRemove() const;

	/// 다음 가젯 충전시간을 반환한다
	int64_t getNextChargeTime() const {
		return _nextChargeTime;
	}

	/// 컨트롤 무제한 여부를 반환한다
	bool isUnLimitedCtrl() const;

	/// 컨트롤중인 액터를 추가한다.
	void addControl(ActorPtr actor);

	/// 컨트롤중인 액터를 삭제한다.
	void removeControl(ActorPtr actor);

	/// 컨트롤중인 액터를 리셋한다
	void resetControls() {
		return _controls.clear();
	}

	/// 컨트롤 중인 액터에게 실행한다.
	void forEachControls(std::function<void(ActorPtr&)>);

	/// 방에서 삭제된다.
	virtual void endPlayPost() override;
	void enterWorld(World& world);
protected:

};
