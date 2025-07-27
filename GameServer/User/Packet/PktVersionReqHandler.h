////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktVersionReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktVersionReq;
class PktVersionReqHandler
    :
    public PacketHandler<User, PktVersionReq>
{
    typedef class PktVersionAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktVersionReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktVersionReq& req, std::shared_ptr<Ack>& ack);

};


