////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktMailListReadReqHandler 의 헤더 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Protocol/PacketHandler.h>

class User;
class PktMailListReadReq;
class PktMailListReadReqHandler
    :
    public PacketHandler<User, PktMailListReadReq>
{
    typedef class PktMailListReadAck Ack;

public:
    /// 패킷을 처리한다
    virtual void onHandler(User& user, PktMailListReadReq& req) override;

private:
    /// 패킷을 처리한다
    void _onHandler(User& user, PktMailListReadReq& req, std::shared_ptr<Ack>& ack);

};


