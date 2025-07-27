////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktCharacterSlotChangeReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktCharacterSlotChangeReq;
class PktCharacterSlotChangeReqHandler
    :
    public PacketHandler<User, PktCharacterSlotChangeReq>
{
    typedef class PktCharacterSlotChangeAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktCharacterSlotChangeReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktCharacterSlotChangeReq& req, std::shared_ptr<Ack>& ack);

};


