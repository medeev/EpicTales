////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktQuestCancelReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktQuestCancelReq;
class PktQuestCancelReqHandler
    :
    public PacketHandler<User, PktQuestCancelReq>
{
    typedef class PktQuestCancelAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktQuestCancelReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktQuestCancelReq& req, std::shared_ptr<Ack>& ack);

};


