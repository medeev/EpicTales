////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		BTNode
///
///	@ date		2024-7-30
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

class NpcAiComponent;

class BTNode;
using BTNodePtr = std::shared_ptr<BTNode>;

class DecoratorNode;
using DecoratorNodePtr = std::shared_ptr<DecoratorNode>;

class TaskNode;
class CompositeNode;
class SequenceNode;
class SelectorNode;

enum class EBTNodeResult {
    Succeeded,
    Failed,
    InProgress
};

enum class EBTFlowAbortMode
{
    None,
    LowerPriority,
    Self,
    Both,
};

inline EBTFlowAbortMode convert(const std::string& str)
{
    static const std::unordered_map<std::string, EBTFlowAbortMode> enumTypes = {
        {"None", EBTFlowAbortMode::None},
        {"LowerPriority", EBTFlowAbortMode::LowerPriority},
        {"Self", EBTFlowAbortMode::Self},
        {"Both", EBTFlowAbortMode::Both}
    };
    auto iter = enumTypes.find(str);
    if (iter != enumTypes.end())
        return iter->second;
    return EBTFlowAbortMode::None;
}

inline const std::string convert(EBTFlowAbortMode type)
{
    static const std::unordered_map<EBTFlowAbortMode, std::string> enumTypes = {
        {EBTFlowAbortMode::None, "None"},
        {EBTFlowAbortMode::LowerPriority, "LowerPriority"},
        {EBTFlowAbortMode::Self, "Self"},
        {EBTFlowAbortMode::Both, "Both", }
    };
    auto iter = enumTypes.find(type);
    if (iter != enumTypes.end())
        return iter->second;
    return std::string("None");
}

inline const std::string convert(EBTNodeResult type)
{
    static const std::unordered_map<EBTNodeResult, std::string> enumTypes = {
        {EBTNodeResult::Succeeded, "Succeeded"},
        {EBTNodeResult::Failed, "Failed"},
        {EBTNodeResult::InProgress, "InProgress"}
    };
    auto iter = enumTypes.find(type);
    if (iter != enumTypes.end())
        return iter->second;
    return std::string("Succeeded");
}
