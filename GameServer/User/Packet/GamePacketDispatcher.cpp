////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 코드를 임의로 수정하지마세요!(자동생성되는 파일입니다.)
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "GamePacketDispatcher.h"
#include "PktCharacterSlotChangeReqHandler.h"
#include "PktChatReqHandler.h"
#include "PktEnvObjCtrlCancelReqHandler.h"
#include "PktEnvObjCtrlStartReqHandler.h"
#include "PktItemEquipChangeReqHandler.h"
#include "PktItemUseReqHandler.h"
#include "PktLoginReqHandler.h"
#include "PktMailListReadReqHandler.h"
#include "PktPingReqHandler.h"
#include "PktPlayerCreateReqHandler.h"
#include "PktPlayerDataReqHandler.h"
#include "PktPlayerDeleteReqHandler.h"
#include "PktPlayerMoveReqHandler.h"
#include "PktQuestCancelReqHandler.h"
#include "PktQuestCompleteReqHandler.h"
#include "PktQuestNpcInteractionReqHandler.h"
#include "PktQuestStartReqHandler.h"
#include "PktStatPointModifyReqHandler.h"
#include "PktVersionReqHandler.h"
#include "PktWorldMoveFinishReqHandler.h"
#include "PktWorldMoveStartReqHandler.h"
#include <Protocol/Struct/PktChat.h>
#include <Protocol/Struct/PktEnvObj.h>
#include <Protocol/Struct/PktGrowth.h>
#include <Protocol/Struct/PktItem.h>
#include <Protocol/Struct/PktMail.h>
#include <Protocol/Struct/PktPlayer.h>
#include <Protocol/Struct/PktQuest.h>
#include <Protocol/Struct/PktRoom.h>
#include <Protocol/Struct/PktSystem.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief 패킷디스패쳐의 생성자입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
GamePacketDispatcher::GamePacketDispatcher()
{
    addPacketHandler(new PktPingReq, new PktPingReqHandler);
    addPacketHandler(new PktVersionReq, new PktVersionReqHandler);
    addPacketHandler(new PktLoginReq, new PktLoginReqHandler);
    addPacketHandler(new PktPlayerDataReq, new PktPlayerDataReqHandler);
    addPacketHandler(new PktPlayerDeleteReq, new PktPlayerDeleteReqHandler);
    addPacketHandler(new PktPlayerCreateReq, new PktPlayerCreateReqHandler);
    addPacketHandler(new PktPlayerMoveReq, new PktPlayerMoveReqHandler);
    addPacketHandler(new PktCharacterSlotChangeReq, new PktCharacterSlotChangeReqHandler);
    addPacketHandler(new PktItemUseReq, new PktItemUseReqHandler);
    addPacketHandler(new PktItemEquipChangeReq, new PktItemEquipChangeReqHandler);
    addPacketHandler(new PktMailListReadReq, new PktMailListReadReqHandler);
    addPacketHandler(new PktEnvObjCtrlStartReq, new PktEnvObjCtrlStartReqHandler);
    addPacketHandler(new PktEnvObjCtrlCancelReq, new PktEnvObjCtrlCancelReqHandler);
    addPacketHandler(new PktWorldMoveStartReq, new PktWorldMoveStartReqHandler);
    addPacketHandler(new PktWorldMoveFinishReq, new PktWorldMoveFinishReqHandler);
    addPacketHandler(new PktChatReq, new PktChatReqHandler);
    addPacketHandler(new PktQuestStartReq, new PktQuestStartReqHandler);
    addPacketHandler(new PktQuestNpcInteractionReq, new PktQuestNpcInteractionReqHandler);
    addPacketHandler(new PktQuestCancelReq, new PktQuestCancelReqHandler);
    addPacketHandler(new PktQuestCompleteReq, new PktQuestCompleteReqHandler);
    addPacketHandler(new PktStatPointModifyReq, new PktStatPointModifyReqHandler);

}


