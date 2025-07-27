////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktQuestCompleteReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktQuestCompleteReq;
class PktQuestCompleteReqHandler
    :
    public PacketHandler<User, PktQuestCompleteReq>
{
    typedef class PktQuestCompleteAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktQuestCompleteReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktQuestCompleteReq& req, std::shared_ptr<Ack>& ack);

};


