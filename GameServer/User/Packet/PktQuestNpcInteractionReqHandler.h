////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktQuestNpcInteractionReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktQuestNpcInteractionReq;
class PktQuestNpcInteractionReqHandler
    :
    public PacketHandler<User, PktQuestNpcInteractionReq>
{
    typedef class PktQuestNpcInteractionAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktQuestNpcInteractionReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktQuestNpcInteractionReq& req, std::shared_ptr<Ack>& ack);

};


