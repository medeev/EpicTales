

#include "Pch.h"
#include "EnvObjStateActivate.h"

#include <Protocol/Struct/PktEnvObj.h>

#include "Actor/EnvObj.h"
#include "Room/Room.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
EnvObjStateActivate::EnvObjStateActivate( FsmComponent& fsm, EnvObj& gadget, EFsmStateType type )
	:
	super( fsm, gadget, type )
{
	_updateTimeInterval = 1000; // 일단 1초정도로
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스테이트가 갱신될 때를 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void EnvObjStateActivate::onUpdate( int64_t curtimeValue )
{
	if ( !_isUpdateTime( curtimeValue ) )
		return;

	_setNextUpdateTime( curtimeValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief	스테이트에 진입할 때를 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void EnvObjStateActivate::onEnter(IState* prevState, const StateInit* init)
{
	super::onEnter(prevState, init);

	_envObj.setEnvObjState( EEnvObjStateType::Activated );

	PktEnvObjStateNotify notify;
	notify.setId( _envObj.getId() );
	notify.setStateInfo( PktEnvObjState( _envObj.getEnvObjState(), _envObj.getRemainCtrlCount() ) );

	if ( auto room = _envObj.getRoom() )
	{
		room->sendToNearExcept( _envObj, notify );
	}
}
