////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktItemUseReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktItemUseReq;
class PktItemUseReqHandler
    :
    public PacketHandler<User, PktItemUseReq>
{
    typedef class PktItemUseAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktItemUseReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktItemUseReq& req, std::shared_ptr<Ack>& ack);

};


