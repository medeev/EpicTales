////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		FSM 컴포넌트 클래스
///
///	@ date		2024-3-21
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "Component/IComponent.h"
#include "FsmTypes.h"
#include "IState.h"

class Actor;
class FsmComponent
	:
	public IComponent
{
public:
	typedef std::map< EFsmStateType, IStatePtr > States;

protected:
	Actor&        _actor;           ///< 부모 엔티티
	IStatePtr     _prevState;       ///< 이전 상태
	IStatePtr     _curState;        ///< 현재상태
	States        _stateMap;        ///< 상태목록
	int64_t       _stateEnterTime;  ///< 현재스테이트 진입시간
	int32_t       _stateElipsedAdd; ///< 스테이트의 진행시간 추가(경과시간이 더된것으로 판단하기위한)
private:
	/// 초기화 한다
	virtual void initialize() override;

	/// 초기화한다.
	virtual bool initializeDB(const DataBaseTarget&) override {
		return true;
	}

	/// 정리 한다
	virtual void finalize() override;

	/// 갱신 한다
	virtual void update(int64_t curTime) override;

	/// 스테이트를 변경한다.
	void _chaseState(EFsmStateType type, const StateInit* init);


public:
	/// 생성자
	FsmComponent(Actor& parent);

	/// 소멸자
	virtual ~FsmComponent();

	/// 상태를 변경한다
	void changeState(EFsmStateType type, const StateInit* init = nullptr);

	/// 상태를 리턴한다
	const IStatePtr getCurState() const {
		return _curState;
	}

	/// 이전상태를 리턴한다.
	const IStatePtr getPrevState() const {
		return _prevState;
	}
	
	/// 현재 스테이트 경과 시간을 반환한다.
	int32_t getElapsedTime(int64_t curTime) const;

	/// 스테이티 경과시간에 추가 시간을 설정한다.
	void setElapsedAddTime(int32_t addTime) {
		_stateElipsedAdd = addTime;
	}

	/// 현재 스테이트에서 작업을 실행한다.
	void doTask(std::function<void(IState*)> task);
};
