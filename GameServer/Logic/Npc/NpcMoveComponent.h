////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		컴포넌트 클래스
///
///	@ date		2024-3-14
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Core/Math/Vector3.h>

#include "Actor/ActorTypes.h"
#include "ActorMoveEnum.h"
#include "Component/IComponent.h"

class EffectComponent;
class FsmComponent;
class PktNpcMoveInfo;
class PktNpcMoveNotify;
class PktActorTeleportNotify;
class Npc;

enum class EMoveToRet
{
	Success,               ///< 이동가능
	LogicError,            ///< 로직에러
	FailNotMoveableEffect, ///< 이동불가효과중
	FailEmptyAccupyToken,  ///< 이동가능한 위치토큰 없음
	FailFindPath,          ///< 길찾기 실패
	FailNotState,          ///< 이동가능 상태아님
};

enum class EMoveToStatus
{
	Moving,               ///< 이동중
	MoveCompleted,        ///< 이동완료
	Stop,                 ///< 정지
};

class NpcMoveComponent : public IComponent
{
public:
	typedef std::list<Vector> PathList;
	typedef std::pair<ActorWeakPtr, EPosTokenType> TargetAndToken;

	enum class EMovementType
	{
		Position,
		ToActor,
	};

protected:
	Npc& _npc;
	PathList _pathPositions; ///< 네비패스 위치목록
	Vector _pathPointEnd; ///< 길찾기 최종위치(바닥좌표)
	Vector _pathPoint; ///< 네비길찾기목적지
	int64_t _sendDebugMsec; ///< 디버깅용 이동위치 패킷 보내는 시간
	EMovementType _movementType; ///< 이동타입(액터or위치)
	EMoveToStatus _moveStatus; ///< 이동상태
	TargetAndToken _targetToken; ///< 내가 가진 타겟의 토큰

	Vector _prevNotifyLocation; ///< 이전위치
	Vector _prevDestPosition; ///< 마지막 목적위치

public:
	/// 생성자
	NpcMoveComponent(Npc& actor);

	/// 소멸자
	virtual ~NpcMoveComponent();

	/// 네비패스위치 목록이 존재하는지 여부를 반환한다.
	bool isEmptyNaviPathList() const {
		return _pathPositions.empty();
	}

	/// 목적지로 이동한다.(바닥위치)
	EMoveToRet moveTo(const Vector& destLocation);

	/// 대상으로 이동한다.
	EMoveToRet moveTo(ActorPtr target);

	/// 텔레포트 한다.(목적지는 바닥위치)
	bool teleport(const Core::Transform& destPosition);

	/// 정지한다.
	void stop();

	/// 다음 경로 목적지를 반환한다.
	const Vector& getPathPoint() const {
		return _pathPoint;
	}

	/// 최종 경로 목적지를 반환한다.
	const Vector& getPathPointEnd() const {
		return _pathPointEnd;
	}

	/// 이동상태를 반환한다.
	const EMoveToStatus getMoveToStatus() const {
		return _moveStatus;
	}

	/// 이동타입을 반환한다.
	const EMovementType getMovementType() const {
		return _movementType;
	}

	/// 현재 따라가는 대상의 토큰반납을 처리한다.
	void returnToken();
private:
	/// 초기화 한다
	virtual void initialize() override;

	/// 정리 한다
	virtual void finalize() override;

	/// 갱신 한다
	virtual void update(int64_t curTime) override;

	/// 거의 같은지 여부를 반환한다.
	bool _equalsNealy(const Vector& src, const Vector& vector) const;

	/// 유효하지 않은 상태인지 여부를 반환한다.
	bool _isValidState() const;

	/// 타겟과 토큰정보를 설정한다.
	void _takeToken(const TargetAndToken& targetToken);

	/// 완료시킨다.
	void _complete();

	/// 이동패킷을 보낸다
	void _sendMoveNotify();

	/// 정지패킷을 보낸다.
	void _sendStopNotify();

};
