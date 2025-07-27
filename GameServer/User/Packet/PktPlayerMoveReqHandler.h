////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPlayerMoveReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktPlayerMoveReq;
class PktPlayerMoveReqHandler
    :
    public PacketHandler<User, PktPlayerMoveReq>
{
    typedef class PktPlayerMoveAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktPlayerMoveReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktPlayerMoveReq& req, std::shared_ptr<Ack>& ack);

};


