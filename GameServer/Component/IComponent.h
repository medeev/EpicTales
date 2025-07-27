////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		컴포넌트 클래스
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms.h>

#include "ComponentTypes.h"

class Entity;
class IComponent
{
	IComponent() = delete;

public:
	/// 생성자
	IComponent(EComponentType type, Entity& entity, EComponentUpdate isUpdate = EComponentUpdate::kNo);
	
	/// 소멸자
	virtual ~IComponent() {}

	/// 초기화 한다
	virtual void initialize() = 0;

	/// 정리 한다
	virtual void finalize() = 0;

	/// 갱신 한다
	virtual void update(int64_t curTime) {}

	/// DB객체로 초기화한다.
	virtual bool initializeDB(const DataBaseTarget&) {
		return true;
	}
	/// 컴포넌트들 DB로드후에 작업한다.
	virtual void initializeDBPost(const DataBaseTarget& db) {
	}

	/// 컴포넌트가 룸진입후 초기화한다
	virtual void beginPlay() {}
};
