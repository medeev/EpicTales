////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktEnvObjCtrlStartReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktEnvObjCtrlStartReq;
class PktEnvObjCtrlStartReqHandler
    :
    public PacketHandler<User, PktEnvObjCtrlStartReq>
{
    typedef class PktEnvObjCtrlStartAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktEnvObjCtrlStartReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktEnvObjCtrlStartReq& req, std::shared_ptr<Ack>& ack);

};


