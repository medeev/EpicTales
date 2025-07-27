////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktPingReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktPingReqHandler.h"

#include <Protocol/Struct/PktSystem.h>

#include "../User.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPingReqHandler::onHandler(User& user, PktPingReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktPingReqHandler::_onHandler(User& user, PktPingReq& req, std::shared_ptr<Ack>& ack)
{
	auto curMSec = Core::Time::GetCurTimeMSec();
	INFO_LOG("recv current time %lld", Core::Time::GetCurTimeMSec());
	if (auto c2sServerMSec = req.getServerMSec())
	{
		int32_t latency = (int32_t)(curMSec - c2sServerMSec);
		if (latency > 0 && latency < 100)
		{
			user.setLatency(latency);
			if (auto player = user.getPlayer())
				player->setLatency(user.getLatency());
		}
	}

	ack->setServerMSec(curMSec);

	user.send(*ack);
	user.setPingTime(curMSec);
}


