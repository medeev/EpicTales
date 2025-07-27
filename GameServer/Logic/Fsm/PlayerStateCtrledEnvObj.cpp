

#include "Pch.h"
#include "PlayerStateCtrledEnvObj.h"

#include "Actor/Player.h"
#include "Room/Room.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	생성자
///
///	@ return 
////////////////////////////////////////////////////////////////////////////////////////////////////
PlayerStateCtrledEnvObj::PlayerStateCtrledEnvObj(FsmComponent& fsm, Player& player, EFsmStateType type)
	:
	super(fsm, player, type),
	_isCtrlEnd(false),
	_ctrlEndTime(0)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	상태에 진입한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerStateCtrledEnvObj::onEnter(IState* prevState, const StateInit* init)
{
	super::onEnter(prevState, init);

	_isCtrlEnd = false;
	_ctrlEndTime = 0;

	auto player = _owner.shared_from_this<Player>();
	ENSURE(player, return);

	if (auto envObj = player->getCtrledEnvObj())
	{
		_ctrlEndTime = Core::Time::GetCurTimeMSec() + envObj->getInfo().getInteractionDurationMSec();
		envObj->addControl(player->shared_from_this());

		PktEnvObjCtrlStartNotify notify;
		notify.setActorId(player->getId());
		notify.setEnvObjId(envObj->getId());

		if (auto room = envObj->getRoom())
			room->sendToNearExcept(_owner, notify);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	상태에서 나간다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerStateCtrledEnvObj::onExit(IState* newState)
{
	auto player = _owner.shared_from_this<Player>();
	ENSURE(player, return);

	if (_isCtrlEnd)
	{
		_controlComplete();
	}
	else
	{
		_controlCompleteFailed();
	}

	player->setCtrledEnvObj(nullptr);

	super::onExit(newState);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	업데이트 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerStateCtrledEnvObj::onUpdate(int64_t curtimeValue)
{
	if (_ctrlEndTime <= curtimeValue)
	{
		_isCtrlEnd = true;

		_fsm.changeState(EFsmStateType::PlayerStateIdle);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  조작을 완료한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerStateCtrledEnvObj::_controlComplete()
{
	auto envObj = _player.getCtrledEnvObj();
	if (!envObj)
		return;

	envObj->removeControl(_player.shared_from_this());

	PktEnvObjCtrlEndNotify notify;
	notify.setActorId(_player.getId());
	notify.setEnvObjId(envObj->getId());

	if (auto room = envObj->getRoom())
		room->sendToNear(_owner, notify);

	if (!envObj->isUnLimitedCtrl())
	{
		if (envObj->getRemainCtrlCount() > 0)
			envObj->setRemainCtrlCount(envObj->getRemainCtrlCount() - 1);

		if (envObj->getRemainCtrlCount() == 0)
		{
			envObj->forEachControls([](ActorPtr& actor)
				{
					if (auto player = actor->shared_from_this<Player>())
						player->getFsmComponent().changeState(EFsmStateType::PlayerStateIdle);
				});

			if (envObj->isRemainEmtpyRemove())
				envObj->getFsmComponent().changeState(EFsmStateType::EnvObjStateDespawn);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief  조작완료를 실패한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void PlayerStateCtrledEnvObj::_controlCompleteFailed()
{
	if (auto gadget = _player.getCtrledEnvObj())
	{
		gadget->removeControl(_player.shared_from_this());

		PktEnvObjCtrlCancelNotify notify;
		notify.setActorId(_player.getId());
		notify.setEnvObjId(gadget->getId());

		if (auto room = gadget->getRoom())
			room->sendToNear(_owner, notify);
	}
}

