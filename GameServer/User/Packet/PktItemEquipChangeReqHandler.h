////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktItemEquipChangeReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktItemEquipChangeReq;
class PktItemEquipChangeReqHandler
    :
    public PacketHandler<User, PktItemEquipChangeReq>
{
    typedef class PktItemEquipChangeAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktItemEquipChangeReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktItemEquipChangeReq& req, std::shared_ptr<Ack>& ack);

};


