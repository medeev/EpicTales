////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		BTNodeManager
///
///	@ date		2024-7-30
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Core/Util/Singleton.h>

#include "BTNodeTypes.h"

class BTNodeManager : public Core::Singleton<BTNodeManager>
{
	friend class Core::Singleton<BTNodeManager>;
	using NodeCreator = std::function<BTNodePtr(const rapidjson::Value&)>;
	using DecoCreator = std::function<DecoratorNodePtr(const rapidjson::Value&, EBTFlowAbortMode)>;

private:
	std::map<uint32_t, BTNodePtr> _behaviorTrees;
	std::unordered_map<std::string, NodeCreator> _nodeCreators;
	std::unordered_map<std::string, DecoCreator> _decoCreators;
public:
	BTNodeManager() {}
	void initialize();
	void finalize();
	BTNode* getBehaviorTree(uint32_t npcInfoId) const;

private:
	void _loadBehaviorTrees(const rapidjson::Document& document);
	BTNodePtr _createNode(const rapidjson::Value& nodeData, const std::string& className);
	DecoratorNodePtr _createDecoratorNode(const rapidjson::Value& nodeData);
	BTNodePtr _parseNode(const rapidjson::Value& nodeData);
};
