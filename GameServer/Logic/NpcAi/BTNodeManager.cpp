#include "Pch.h"
#include "BTNodeManager.h"

#include <Core/Log/Log.h>
#include <Core/Stream/FileReader.h>
#include <Core/Stream/FileReader.h>
#include <Core/Util/AppInstance.h>
#include <Core/Util/FileUtil.h>
#include <Data/Info/InfoEnumsConv.h>
#include <Data/Info/SkillInfo.h>

#include "BTFuncDecoration.h"
#include "BTFuncTask.h"
#include "BTNode.h"
#include "BTNodeTypes.h"
#include "NpcAiComponent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  행동트리를 초기화한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void BTNodeManager::initialize()
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  정리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void BTNodeManager::finalize()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  행동트리를 로드한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void BTNodeManager::_loadBehaviorTrees(const rapidjson::Document& document)
{
	for (rapidjson::SizeType i = 0; i < document.Size(); ++i) {
		const auto& doc = document[i];
		std::string path = doc["Path"].GetString();

		BTNodePtr node = _parseNode(doc);
		node->buildObserveAbort();

		if (doc.HasMember("NpcInfoId"))
		{
			const auto& npcNode = doc["NpcInfoId"];
			if (npcNode.IsArray())
			{
				for (rapidjson::SizeType npc_i = 0; npc_i < npcNode.Size(); ++npc_i)
				{
					auto npcInfoId = npcNode[npc_i].GetInt();
					_behaviorTrees[npcInfoId] = node;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  행동트리를 가져온다
////////////////////////////////////////////////////////////////////////////////////////////////////
BTNode* BTNodeManager::getBehaviorTree(uint32_t npcInfoId) const
{
	auto it = _behaviorTrees.find(npcInfoId);
	if (it != _behaviorTrees.end()) {
		return it->second.get();
	}
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  노드를 파싱한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
BTNodePtr BTNodeManager::_parseNode(const rapidjson::Value& nodeData)
{
	std::string className = nodeData["ClassName"].GetString();
	BTNodePtr node = _createNode(nodeData, className);

	if (auto composite = std::dynamic_pointer_cast<CompositeNode>(node))
	{
		if (nodeData.HasMember("Decorations")) {
			const auto& decorations = nodeData["Decorations"];
			for (rapidjson::SizeType i = 0; i < decorations.Size(); ++i) {
				const auto& decoNodeData = decorations[i];
				if (auto decoNode = _createDecoratorNode(decoNodeData))
					composite->addDecorator(decoNode);
			}
		}

		if (nodeData.HasMember("Children")) {
			const auto& children = nodeData["Children"];
			for (rapidjson::SizeType i = 0; i < children.Size(); ++i) {
				auto childNode = _parseNode(children[i]);
				if (!childNode)
					continue;
				composite->addChild(childNode);
			}
		}
	}
	else if (auto taskNode = std::dynamic_pointer_cast<TaskNode>(node))
	{
		auto& checkCommon = taskNode->getCheckCommonData();
		if (nodeData.HasMember("ExcuteLimit"))
			checkCommon.excuteLimit = nodeData["ExcuteLimit"].GetInt();
		if (nodeData.HasMember("LimitReturn"))
			checkCommon.limitReturn = nodeData["LimitReturn"].GetBool();
		if (nodeData.HasMember("CoolTimeMSec"))
			checkCommon.coolTimeMSec = nodeData["CoolTimeMSec"].GetInt();

		if (nodeData.HasMember("Decorations")) {
			const auto& decorations = nodeData["Decorations"];
			for (rapidjson::SizeType i = 0; i < decorations.Size(); ++i) {
				const auto& decoNodeData = decorations[i];
				std::string decoratorClass = decoNodeData["ClassName"].GetString();
				if (auto decoNode = _createDecoratorNode(decoNodeData))
					taskNode->addDecorator(decoNode);
			}
		}
	}

	return node;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  행동트리 노드를 생성한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
BTNodePtr BTNodeManager::_createNode(const rapidjson::Value& nodeData, const std::string& className)
{
	auto nodeName = nodeData["NodeName"].GetString();

	auto it = _nodeCreators.find(className);
	if (it == _nodeCreators.end()) {
		//WARN_LOG("BT Class not exist [className: %s, nodeName:%s]", className.c_str(), nodeName);
		return nullptr;
	}

	auto node = it->second(nodeData);
	if (!node)
	{
		WARN_LOG("BT Class exist but parse failed. [className: %s, nodeName:%s]", className.c_str(), nodeName);
		return nullptr;
	}

	node->setNodeName(nodeName);
	return node;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  행동트리 데코레이터를 생성한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
DecoratorNodePtr BTNodeManager::_createDecoratorNode(const rapidjson::Value& nodeData)
{
	std::string decoratorClass = nodeData["ClassName"].GetString();
	std::string abortModeStr = nodeData["AbortMode"].GetString();
	std::string nodeName = nodeData["NodeName"].GetString();

	bool inverseCondition = false;
	if (nodeData.HasMember("InverseCondition"))
		inverseCondition = nodeData["InverseCondition"].GetBool();

	EBTFlowAbortMode abortMode = convert(abortModeStr);

	auto it = _decoCreators.find(decoratorClass);
	if (it == _decoCreators.end())
	{
		WARN_LOG("BT Decorator not exist [name : %s]", decoratorClass.c_str());
		return nullptr;
	}

	auto decoNode = it->second(nodeData, abortMode);
	if (!decoNode)
	{
		WARN_LOG("BT Decorator exist but parse failed [name : %s]", decoratorClass.c_str());
		return nullptr;
	}

	decoNode->setNodeName(nodeName);
	decoNode->setInverse(inverseCondition);

	return decoNode;
}
