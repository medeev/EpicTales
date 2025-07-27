

#include "Pch.h"
#include "EnvObjStateSpawn.h"

#include <Data/Info/EnvObjInfo.h>

#include "Actor/EnvObj.h"
#include "Actor/Player.h"
#include "FsmComponent.h"
#include "Room/World.h"

EnvObjStateSpawn::EnvObjStateSpawn(FsmComponent& fsm, EnvObj& gadget, EFsmStateType type)
	:
	super(fsm, gadget, type)
{

}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스폰상태에 진입한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void EnvObjStateSpawn::onEnter(IState* prevState, const StateInit* init)
{
	super::onEnter(prevState, init);

	_envObj.setEnvObjState(_envObj.getInfo().getInitStateType());

	PktEnvObjSpawnNotify notify;

	PktEnvObjInfo pktEnvObj;
	_envObj.exportTo(pktEnvObj);
	notify.getEnvObjInfos().emplace_back(std::move(pktEnvObj));

	if (auto room = _envObj.getRoom())
		room->sendToNear(_envObj, notify);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스테이를 업데이트한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void EnvObjStateSpawn::onUpdate(int64_t curtimeValue)
{
	if (_envObj.getInfo().getInitStateType() == EEnvObjStateType::Deactivated)
		_fsm.changeState(EFsmStateType::EnvObjStateDeactivate);
	else
		_fsm.changeState(EFsmStateType::EnvObjStateActivate);
}

