////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPingReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktPingReq;
class PktPingReqHandler
    :
    public PacketHandler<User, PktPingReq>
{
    typedef class PktPingAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktPingReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktPingReq& req, std::shared_ptr<Ack>& ack);

};


