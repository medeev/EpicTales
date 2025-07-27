////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief        BTRunningTaskData
///
/// @date         2024-8-8
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "BTNodeTypes.h"

class BTNode;
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Tree에서 Running중인 노드에 대한 정보
////////////////////////////////////////////////////////////////////////////////////////////////////
class BTRunningTaskData
{
public:
    BTRunningTaskData() :
        _runningTask(nullptr),
        _latentResult(EBTNodeResult::InProgress)
    {
    }

    void reset();

    int32_t getCompositeChildIndex(const BTNode* composite) const;
    void setCompositChildIndex(const BTNode* composite, int32_t index);
    void setRunningTask(const TaskNode* taskNode);
    bool isRunningTask(const TaskNode* taskNode) const {
        return taskNode == _runningTask;
    }
    bool isRunning() const {
        return _runningTask != nullptr;
    }
    void setLatentResult(EBTNodeResult ret) {
        _latentResult = ret;
    }
    EBTNodeResult getLatentResult() const {
        return _latentResult;
    }

protected:
    std::map<const BTNode*, int32_t> _compositeChildIndexes;
    const TaskNode* _runningTask;
    EBTNodeResult _latentResult;
};
