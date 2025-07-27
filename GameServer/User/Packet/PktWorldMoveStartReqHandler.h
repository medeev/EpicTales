////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktWorldMoveStartReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktWorldMoveStartReq;
class PktWorldMoveStartReqHandler
    :
    public PacketHandler<User, PktWorldMoveStartReq>
{
    typedef class PktWorldMoveStartAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktWorldMoveStartReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktWorldMoveStartReq& req, std::shared_ptr<Ack>& ack);

};


