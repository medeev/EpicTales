////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		유저 관리자
///
///	@ date		2024-3-6 
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "User/UserTypes.h"
#include <Asio/Service/AcceptorEventListener.h>
#include <ThreadSafeContainer/TSMap.h>

class UserManager
	:
	public Core::Singleton<UserManager>,
	public Core::TSMap<int64_t, UserPtr>,
	public Core::AcceptorEventListener
{
	friend class Core::Singleton<UserManager>;
private:
	int32_t	_connectionId;

private:
	UserManager() : _connectionId(0) {}
public:

	/// 연결 대기할 객체를 얻는 이벤트
	virtual Core::ConnectionPtr onAllocateConnection() override;

	/// 연결 성공 이벤트
	virtual void onAccepted(Core::Connection& connection) override;

	/// 연결 실패 이벤트
	virtual void onAcceptFailed(Core::Connection& connection, const AsioErrorCode& error) override;

protected:
};
