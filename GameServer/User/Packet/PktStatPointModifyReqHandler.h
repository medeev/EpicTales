////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktStatPointModifyReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktStatPointModifyReq;
class PktStatPointModifyReqHandler
    :
    public PacketHandler<User, PktStatPointModifyReq>
{
    typedef class PktStatPointModifyAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktStatPointModifyReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktStatPointModifyReq& req, std::shared_ptr<Ack>& ack);

};


