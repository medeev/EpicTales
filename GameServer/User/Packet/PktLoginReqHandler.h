////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktLoginReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktLoginReq;
class PktLoginReqHandler
    :
    public PacketHandler<User, PktLoginReq>
{
    typedef class PktLoginAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktLoginReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktLoginReq& req, std::shared_ptr<Ack>& ack);

};


