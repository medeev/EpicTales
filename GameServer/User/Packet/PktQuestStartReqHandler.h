////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktQuestStartReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktQuestStartReq;
class PktQuestStartReqHandler
    :
    public PacketHandler<User, PktQuestStartReq>
{
    typedef class PktQuestStartAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktQuestStartReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktQuestStartReq& req, std::shared_ptr<Ack>& ack);

};


