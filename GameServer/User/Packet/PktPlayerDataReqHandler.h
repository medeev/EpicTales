////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPlayerDataReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktPlayerDataReq;
class PktPlayerDataReqHandler
    :
    public PacketHandler<User, PktPlayerDataReq>
{
    typedef class PktPlayerDataAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktPlayerDataReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktPlayerDataReq& req, std::shared_ptr<Ack>& ack);

};


