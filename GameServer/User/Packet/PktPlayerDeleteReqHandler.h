////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPlayerDeleteReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktPlayerDeleteReq;
class PktPlayerDeleteReqHandler
    :
    public PacketHandler<User, PktPlayerDeleteReq>
{
    typedef class PktPlayerDeleteAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktPlayerDeleteReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktPlayerDeleteReq& req, std::shared_ptr<Ack>& ack);

};


