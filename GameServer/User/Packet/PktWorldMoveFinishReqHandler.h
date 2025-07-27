////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktWorldMoveFinishReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktWorldMoveFinishReq;
class PktWorldMoveFinishReqHandler
    :
    public PacketHandler<User, PktWorldMoveFinishReq>
{
    typedef class PktWorldMoveFinishAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktWorldMoveFinishReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktWorldMoveFinishReq& req, std::shared_ptr<Ack>& ack);

};


