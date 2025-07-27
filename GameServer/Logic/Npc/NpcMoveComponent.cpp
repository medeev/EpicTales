#include "Pch.h"
#include "NpcMoveComponent.h"

#include <Core/Math/Numeric.h>
#include <Core/Util/StringUtil.h>
#include <Core/Util/Time.h>
#include <Data/Info/WorldInfo.h>
#include <Protocol/PktCommon.h>
#include <Protocol/Struct/PktNpc.h>
#include <Protocol/Struct/PktShape.h>

#include "Actor/Npc.h"
#include "Config.h"
#include "Logic/Fsm/FsmComponent.h"
#include "Room/World.h"
#include "Util/UtilExporter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcMoveComponent::NpcMoveComponent(Npc& npc)
	:
	IComponent(EComponentType::NpcMovement, npc, EComponentUpdate::kYes),
	_npc(npc),
	_sendDebugMsec(0)
{
	_targetToken.first.reset();
	_targetToken.second = EPosTokenType::None;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
NpcMoveComponent::~NpcMoveComponent()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcMoveComponent::initialize()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		정리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcMoveComponent::finalize()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	목적지로 이동한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
EMoveToRet NpcMoveComponent::moveTo(const Vector& destLocation)
{
	if (!_isValidState())
		return EMoveToRet::FailNotState;

	auto world = _npc.getWorld();
	if (!world)
		return EMoveToRet::LogicError;

	if (_moveStatus == EMoveToStatus::Moving)
		if (_movementType == EMovementType::ToActor)
			returnToken();

	_movementType = EMovementType::Position;

	if (_equalsNealy(_npc.getLocation(), destLocation))
	{
		_pathPoint = destLocation - Vector(0, 0, _npc.getHalfHeight());
		_pathPointEnd = _pathPoint;
		_pathPositions.clear();

		_sendMoveNotify();

		return EMoveToRet::Success;
	}

	const auto& worldInfo = world->getInfo();

	PathFindingResult pathFind;
	if (!worldInfo.findPath(_npc.getLocation(), destLocation, pathFind))
		return EMoveToRet::FailFindPath;

	const auto naviPaths = pathFind.getPaths();
	if (naviPaths.empty())
		return EMoveToRet::FailFindPath;
	
	_pathPositions = naviPaths;

	const auto& pathPos = _pathPositions.front();

	_pathPoint = pathPos;
	_pathPointEnd = _pathPositions.back();
	_pathPositions.pop_front();

	auto rootLocation = _npc.getRootLocation();
	rootLocation.z = _pathPoint.z;

	// 네비 패스가 현재위치라면 바로  다음위치를 설정한다
	if (_equalsNealy(_pathPoint, rootLocation))
	{
		if (!_pathPositions.empty())
		{
			const auto& pathPos = _pathPositions.front();
			_pathPoint = pathPos;
			_pathPositions.pop_front();
		}
	}

	auto npcToPathVector =
		_pathPoint - rootLocation;

	if (!npcToPathVector.isZero())
		_npc.setDirection(npcToPathVector.getSafeNormal2D());

	_sendMoveNotify();

	_moveStatus = EMoveToStatus::Moving;

	if (Config::GetDebugNpcMove())
	{
		Vector prev;
		PktShapeNotify notify;

		int32_t i = 0;
		for (const auto& path : naviPaths)
		{
			if (i == 0)
			{
			}
			else
			{
				notify.getLineList().emplace_back(
					prev, path, 0.8f/*duration*/, 2.f, PktColorBlue, true);
				notify.getTextList().emplace_back(
					path,
					STRINGUTIL_FORMAT("%d,%d,%d", (int32_t)path.x, (int32_t)path.y, (int32_t)path.z), 0.8f, 1.f, PktColorBlue);
			}

			prev = path;
			i++;
		}
		world->sendToNear(_npc, notify);
	}

	return EMoveToRet::Success;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	대상으로 이동한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
EMoveToRet NpcMoveComponent::moveTo(ActorPtr target)
{
	if (!_isValidState())
		return EMoveToRet::FailNotState;

	auto world = _npc.getWorld();
	if (!world)
		return EMoveToRet::LogicError;

	if (!target)
		return EMoveToRet::LogicError;

	returnToken();

	float radiusPlus = target->getRadius() + _npc.getRadius();

	Vector findLocation;
	EPosTokenType posToken;

	if (target->findNearestOccupyToken(
		world->getTime(), _npc.getRootLocation(), radiusPlus, findLocation, posToken))
	{
		auto targetPos = target->getRootLocation();
		targetPos.z = 0;

		auto curPos = _npc.getRootLocation();
		curPos.z = 0;

		auto target2Cur = curPos - targetPos;
		auto target2Location = findLocation - targetPos;

		// 타겟을 뚫고 가지 않게 하기위해서 타겟앞쪽위치면 그냥 가고, 뒤쪽이면 목적지로가지말고 측면90도 위치로 돌아가게 설정한다.
		// 1) 찾은 위치가 타겟의 뒤쪽인지 판단(내적이 0보다작음판단: 뒤쪽)
		if (Vector::Dot(target2Cur, target2Location) < 0)
		{
			// 2 ) 외적이 Up백터와의 내적값이 0보다 작으면 오른쪽, 크면 왼쪽
			if (Vector::Dot(Vector::UpVector, Vector::Cross(target2Location, target2Cur)) < 0)
			{
				auto rightDir = target2Cur.perpendicular();
				rightDir.normalize();

				// 우측으로 돌아가는 끝 위치를 구한다
				rightDir = rightDir * (_npc.getRadius() + target->getRadius()) * 2.f;
				findLocation = targetPos + rightDir;
			}
			else
			{
				auto leftDir = -target2Cur.perpendicular();
				leftDir.normalize();

				// 좌측으로 돌아가는 끝 위치를 구한다
				leftDir = leftDir * (_npc.getRadius() + target->getRadius()) * 2.f;
				findLocation = targetPos + leftDir;
			}
		}
	}
	else
	{
		return EMoveToRet::FailEmptyAccupyToken;
	}

	auto capsuleLocation = findLocation;
	capsuleLocation.z = _npc.getLocation().z;

	PathFindingResult pathFind;
	if (!world->getInfo().findPath(_npc.getLocation(), capsuleLocation, pathFind))
		return EMoveToRet::FailFindPath;
	
	const auto& naviPaths = pathFind.getPaths();
	if (naviPaths.empty())
		return EMoveToRet::FailFindPath;

	_takeToken(std::make_pair(target, posToken));
	
	_movementType = EMovementType::ToActor;

	_pathPositions = naviPaths;

	const auto& pathPos = _pathPositions.front();

	_pathPoint = pathPos;
	_pathPointEnd = _pathPositions.back();
	_pathPositions.pop_front();

	auto rootLocation = _npc.getRootLocation();
	rootLocation.z = _pathPoint.z;

	// 네비 패스가 현재위치라면 바로  다음위치를 설정한다
	while (_equalsNealy(_pathPoint, rootLocation))
	{
		if (_pathPositions.empty())
			break;
		
		const auto& pathPos = _pathPositions.front();
		_pathPoint = pathPos;
		_pathPositions.pop_front();
	}

	auto npcToPathVector =
		_pathPoint - rootLocation;
	if (!npcToPathVector.isZero())
		_npc.setDirection(npcToPathVector.getSafeNormal2D());

	_moveStatus = EMoveToStatus::Moving;

	_sendMoveNotify();

	if (Config::GetDebugNpcMove())
	{
		Vector prev;
		PktShapeNotify notify;

		int32_t i = 0;
		for (const auto& path : naviPaths)
		{
			if (i == 0)
			{
			}
			else
			{
				notify.getLineList().emplace_back(
					prev, path, 0.8f/*duration*/, 2.f, PktColorBlue, true);
				notify.getTextList().emplace_back(
					path,
					STRINGUTIL_FORMAT("%d,%d,%d", (int32_t)path.x, (int32_t)path.y, (int32_t)path.z), 0.8f, 1.f, PktColorBlue);
			}

			prev = path;
			i++;
		}
		world->sendToNear(_npc, notify);
	}

	return EMoveToRet::Success;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		텔레포트 한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcMoveComponent::teleport(const Core::Transform& transform)
{
	_npc.setLocation(transform.location());
	_npc.setDirection(transform.rotator().vector());
	stop();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	업데이트한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcMoveComponent::update(int64_t curTime)
{
	auto room = _npc.getRoom();
	if (!room)
		return;

	float deltaSec = room->getDeltaSec();

	if (_moveStatus != EMoveToStatus::Moving)
		return;

	if (_pathPoint.isZero())
		return;

	auto prevRootLocation = _npc.getRootLocation();
	prevRootLocation.z = _pathPoint.z;

	auto npcToPathVector = _pathPoint - prevRootLocation;
	auto moveVector = npcToPathVector.getSafeNormal();
	moveVector = moveVector * (float)240 * deltaSec;
	if (moveVector.lengthSq() > npcToPathVector.lengthSq())
		moveVector = npcToPathVector;

	if (!moveVector.isZero())
	{
		auto newLocation = prevRootLocation + moveVector;
		newLocation.z += _npc.getHalfHeight();
		_npc.setLocation(newLocation);
		_npc.setDirection(moveVector.getSafeNormal2D());
	}

	// 네비 패스 다음위치를 설정한다
	auto curRootLocation = _npc.getRootLocation();
	bool isNear = false;
	while (_equalsNealy(_pathPoint, curRootLocation))
	{
		if (_pathPositions.empty())
			break;

		isNear = true;

		const auto& pathPos = _pathPositions.front();
		_pathPoint = pathPos;
		_pathPositions.pop_front();
	}

	if (isNear)
		_sendMoveNotify();

	// 네비 패스 최종위치 도달시 처리한다
	if (_equalsNealy(_pathPointEnd, curRootLocation))
	{
		_complete();
	}
	
	if (Config::GetDebugNpcMove())
	{
		PktShapeNotify notify;
		UtilExporter::ExportTo(_npc.getCollision(), notify, PktColorSilver);

		if (const auto room = _npc.getRoom())
			room->sendToNear(_npc, notify);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	정지한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcMoveComponent::stop()
{
	if (
		_moveStatus == EMoveToStatus::Stop ||
		_moveStatus == EMoveToStatus::MoveCompleted)
		return;

//	returnToken();

	_pathPositions.clear();
	_pathPoint = _npc.getRootLocation();
	_pathPointEnd = _npc.getRootLocation();
	_moveStatus = EMoveToStatus::Stop;

	_sendStopNotify();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  목적지 도착처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcMoveComponent::_complete()
{
//	returnToken();

	_pathPositions.clear();
	_pathPoint = Vector::ZeroVector;
	_pathPointEnd = _npc.getRootLocation();
	_moveStatus = EMoveToStatus::MoveCompleted;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	이동에서 체크하는 거의 같은지의 여부를 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool NpcMoveComponent::_equalsNealy(const Vector& src, const Vector& vector) const
{
	return
		Core::Numeric::Abs(src.x - vector.x) <= 5.f &&
		Core::Numeric::Abs(src.y - vector.y) <= 5.f/* &&
		Core::Numeric::Abs(src.z - vector.z) <= 5.f*/;
}

bool NpcMoveComponent::_isValidState() const
{
	return 
		_npc.getState() == EFsmStateType::NpcStateChase || 
		_npc.getState() == EFsmStateType::NpcStateReturn || 
		_npc.getState() == EFsmStateType::NpcStateRoaming ||
		_npc.getState() == EFsmStateType::NpcStateMoveTo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  이동 패킷을 보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcMoveComponent::_sendMoveNotify()
{
	PktNpcMoveNotify notify;
	notify.setLocation(_npc.getLocation());
	notify.setYaw((uint16_t)Vector::GetYaw(_npc.getDirection()));
	notify.setDestLocation(_pathPoint);
	notify.setNpcId(_npc.getId());

	if (auto room = _npc.getRoom())
		room->sendToNearExcept(_npc, notify);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  이동 패킷을 보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcMoveComponent::_sendStopNotify()
{
	PktNpcMoveNotify notify;
	notify.setLocation(_npc.getLocation());
	notify.setDestLocation(_npc.getLocation());
	notify.setYaw((uint16_t)Vector::GetYaw(_npc.getDirection()));
	notify.setNpcId(_npc.getId());
	notify.setIsStop(true);

	if (auto room = _npc.getRoom())
		room->sendToNearExcept(_npc, notify);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	현재 따라가는 대상의 토큰반납을 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcMoveComponent::returnToken()
{
	ActorPtr prevTarget = _targetToken.first.lock();
	EPosTokenType prevToken = _targetToken.second;

	if (prevTarget && prevTarget->getRoom() == _npc.getRoom())
		prevTarget->addLocationToken((int32_t)prevToken);

	_targetToken.first.reset();
	_targetToken.second = EPosTokenType::None;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  타겟과 토큰정보를 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void NpcMoveComponent::_takeToken(const TargetAndToken& targetToken)
{
	auto targetWeak = targetToken.first;
	auto token = targetToken.second;

	if (auto target = targetWeak.lock())
		target->removeLocationToken((int32_t)token);

	_targetToken = targetToken;
}
