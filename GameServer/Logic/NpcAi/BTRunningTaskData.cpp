#include "Pch.h"
#include "BTRunningTaskData.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  초기화한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void BTRunningTaskData::reset()
{
    _compositeChildIndexes.clear();
    _runningTask = nullptr;
    _latentResult = EBTNodeResult::InProgress;
}

int32_t BTRunningTaskData::getCompositeChildIndex(const BTNode* composite) const
{
    if (!isRunning())
        return 0;

    auto iter = _compositeChildIndexes.find(composite);
    if (iter == _compositeChildIndexes.end())
        return 0;
    return iter->second;
}

void BTRunningTaskData::setCompositChildIndex(const BTNode* composite, int32_t index)
{
    _compositeChildIndexes[composite] = index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Running Task를 설정한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void BTRunningTaskData::setRunningTask(const TaskNode* taskNode)
{
    if (taskNode)
        _latentResult = EBTNodeResult::InProgress;

    _runningTask = taskNode;
}
