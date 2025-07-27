#pragma once

#include "Actor/ActorTypes.h"
#include "Component/IComponent.h"
#include "TriggerType.h"

class World;
class TriggerComponent
	:
	public IComponent
{
public:
	static EComponentType GetComponentType() {
		return EComponentType::Trigger;
	}

private:
	World& _world; ///< 월드정보
	int64_t _nextUpdate; ///< 다음업데이트
	std::unordered_map<std::string, TriggerPtr> _triggers;
public:
	/// 생성자
	TriggerComponent(World& world);

	/// 소멸자
	virtual ~TriggerComponent();

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// 정리 한다
	virtual void finalize() override;

	/// 갱신 한다
	virtual void update(int64_t curTime) override;

public:
	const TriggerPtr find(const std::string& name) const;
};
