////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief PktChatReqHandler 의 소스 파일입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "PktChatReqHandler.h"

#include <Protocol/Struct/PktChat.h>

#include "../User.h"
#include "Actor/Player.h"
#include "Cheat/CheatManager.h"
#include "Room/Room.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktChatReqHandler::onHandler(User& user, PktChatReq& req)
{
	std::shared_ptr<Ack> ack = std::make_shared<Ack>();
	ack->setReqKey(req.getReqKey());
	_onHandler(user, req, ack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷을 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void PktChatReqHandler::_onHandler(User& user, PktChatReq& req, std::shared_ptr<Ack>& ack)
{
	auto player = user.getPlayer();
	if (!player)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	auto room = player->getRoom();
	if (!room)
	{
		ack->setResult(EResultCode::InvalidState);
		user.send(*ack);
		return;
	}

	auto message = req.getChat();
	if (message.empty())
	{
		ack->setResult(EResultCode::ParamError);
		user.send(*ack);
		return;
	}

	if (CheatManager::GetInstance().handleChat(player, message))
	{
		user.send(*ack);
		return;
	}

	PktChatNotify notify;
	notify.getChatInfo().setChat(message);
	room->sendToNear(*player, notify);

	user.send(*ack);
}


