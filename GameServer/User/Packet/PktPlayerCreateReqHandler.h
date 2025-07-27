////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPlayerCreateReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktPlayerCreateReq;
class PktPlayerCreateReqHandler
    :
    public PacketHandler<User, PktPlayerCreateReq>
{
    typedef class PktPlayerCreateAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktPlayerCreateReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktPlayerCreateReq& req, std::shared_ptr<Ack>& ack);

};


