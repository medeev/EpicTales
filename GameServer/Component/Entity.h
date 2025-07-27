////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		엔티티 클래스
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <list>
#include <map>

#include "ComponentTypes.h"

class IComponent;
class Entity
{
	friend IComponent;

	/// 컴포넌트 배열 타입 정의
	typedef std::map<EComponentType, IComponent*> Components;

protected:
	Components _components;  ///< 컴포넌트 맵
	std::list<IComponent*> _updates;///< 업데이트되는 컴포넌트

public:
	/// 생성자
	Entity()
	{
	}

	/// 소멸자
	virtual ~Entity()
	{
		_components.clear();
		_updates.clear();
	}


	/// 컴포넌트를 추가한다.
	void addComponent(EComponentType type, IComponent* self, EComponentUpdate isUpdate);

	/// 스레드가 올바른지 체크한다
	virtual bool checkThread() const {
		return true;
	}

	/// 컴포넌트들을 초기환한다.
	void initComponents();

	/// 컴포넌트들을 해제한다.
	void finalizeComponents();

	/// 컴포넌트가 룸진입후 처리한다.
	void componentBeginPlay();

protected:
	/// 갱신 한다
	void updateComponent(int64_t curTime);

	
};
