////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktChatReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktChatReq;
class PktChatReqHandler
    :
    public PacketHandler<User, PktChatReq>
{
    typedef class PktChatAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktChatReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktChatReq& req, std::shared_ptr<Ack>& ack);

};


