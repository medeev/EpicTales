////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktEnvObjCtrlCancelReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktEnvObjCtrlCancelReq;
class PktEnvObjCtrlCancelReqHandler
    :
    public PacketHandler<User, PktEnvObjCtrlCancelReq>
{
    typedef class PktEnvObjCtrlCancelAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktEnvObjCtrlCancelReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktEnvObjCtrlCancelReq& req, std::shared_ptr<Ack>& ack);

};


