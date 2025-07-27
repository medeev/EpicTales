#include "Pch.h"
#include "BTNode.h"

#include "Actor/Npc.h"
#include "BTRunningTaskData.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  서브트리의 모든 Task(Leaf)노드를 저장하는 함수
////////////////////////////////////////////////////////////////////////////////////////////////////
void collectNodes(BTNode* root, std::vector<BTNode*>& result)
{
    if (root == nullptr) {
        return;
    }

    if (auto composit = dynamic_cast<CompositeNode*>(root))
    {
        for (auto child : composit->getChildren()) {
            collectNodes(child.get(), result);
        }
    }
    else
    {
        result.push_back(root);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  특정 노드의 모든 오른쪽 노드를 반환하는 함수
////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<BTNode*> getRightNodes(BTNode* target)
{
    std::vector<BTNode*> result;
    auto current = target;

    while (current != nullptr) {
        bool rightSibling = false;
        auto parent = current->getParent();
        if (parent != nullptr) {
            if (auto composit = dynamic_cast<CompositeNode*>(parent))
            {
                for (auto sibling : composit->getChildren()) {
                    if (rightSibling) {
                        collectNodes(sibling.get(), result);
                    }
                    if (sibling.get() == current) {
                        rightSibling = true;
                    }
                }
            }
        }
        current = parent;
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  TaskNode를 실행한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
EBTNodeResult TaskNode::execute(Npc& npc, BTRunningTaskData& runningTaskData) const
{
    /// 옵저버 취소는 Running도 캔슬시킬 수 있다.
    for (const auto& observe : _decoratorObserves)
    {
        if (observe->checkCondition(npc))
        {
            auto ret = EBTNodeResult::Failed;
            npc.getNpcAiComponent().onTaskResult(*this, ret);
            return ret;
        }
    }

    /// 현재 Task가 Running중인 Task일때 BT트리중에 하나의 Task만 running상태로 진입한다.(병렬Task고려안한다.)
    if (runningTaskData.isRunningTask(this))
    {
        /// 예약된 결과가 있으면 해당결과를 반환한다.
        auto ret = runningTaskData.getLatentResult();
        if (ret != EBTNodeResult::InProgress)
            npc.getNpcAiComponent().onTaskResult(*this, ret);

        return ret;
    }
    else
    {
        EBTNodeResult ret;
        if (!npc.getNpcAiComponent().checkEnableTask(*this, ret))
        {
            npc.getNpcAiComponent().onTaskResult(*this, ret);
            return ret;
        }
    }

    for (const auto& decorator : _decorators) {
        if (!decorator->checkCondition(npc)) {
            auto ret = EBTNodeResult::Failed;
            npc.getNpcAiComponent().onTaskResult(*this, ret);
            return ret;
        }
    }

    auto ret = _task(*this, npc);
    if (ret == EBTNodeResult::InProgress)
        runningTaskData.setRunningTask(this);
    
    npc.getNpcAiComponent().onTaskResult(*this, ret);

    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  TaskNode의 옵저버Abort Condition에 대해서 생성한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void TaskNode::buildObserveAbort()
{
    for (auto decorator : _decorators)
    {
        if (decorator->getAbortType() == EBTFlowAbortMode::LowerPriority)
        {
            auto tasks = getRightNodes(this);
            for (auto node : tasks)
            {
                if (auto task = dynamic_cast<TaskNode*>(node))
                    task->addObserveAbortCondition(decorator.get());
            }
        }
        else if (decorator->getAbortType() == EBTFlowAbortMode::Self)
        {
            std::vector<BTNode*> tasks;
            collectNodes(this, tasks);
            for (auto node : tasks)
            {
                if (auto task = dynamic_cast<TaskNode*>(node))
                    task->addObserveAbortCondition(decorator.get());
            }
        }
        else if (decorator->getAbortType() == EBTFlowAbortMode::Both)
        {
            {
                auto tasks = getRightNodes(this);
                for (auto node : tasks)
                {
                    if (auto task = dynamic_cast<TaskNode*>(node))
                        task->addObserveAbortCondition(decorator.get());
                }
            }
            {
                std::vector<BTNode*> tasks;
                collectNodes(this, tasks);
                for (auto node : tasks)
                {
                    if (auto task = dynamic_cast<TaskNode*>(node))
                        task->addObserveAbortCondition(decorator.get());
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  출력해본다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void TaskNode::print(Npc& npc, EBTNodeResult result) const
{
    npc.getNpcAiComponent().printTask(*this, result);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  컴포지션노드의 옵저버어볼트에 대한 Condition을 생성한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CompositeNode::buildObserveAbort()
{
    for (auto decorator : _decorators)
    {
        if (decorator->getDecoType() != DecoratorNode::EDecoType::kIfCheck)
            continue;

        if (decorator->getAbortType() == EBTFlowAbortMode::LowerPriority)
        {
            auto tasks = getRightNodes(this);
            for (auto node : tasks)
            {
                if (auto task = dynamic_cast<TaskNode*>(node))
                    task->addObserveAbortCondition(decorator.get());
            }
        }
        else if (decorator->getAbortType() == EBTFlowAbortMode::Self)
        {
            std::vector<BTNode*> tasks;
            collectNodes(this, tasks);
            for (auto node : tasks)
            {
                if (auto task = dynamic_cast<TaskNode*>(node))
                    task->addObserveAbortCondition(decorator.get());
            }
        }
        else if (decorator->getAbortType() == EBTFlowAbortMode::Both)
        {
            {
                auto tasks = getRightNodes(this);
                for (auto node : tasks)
                {
                    if (auto task = dynamic_cast<TaskNode*>(node))
                        task->addObserveAbortCondition(decorator.get());
                }
            }
            {
                std::vector<BTNode*> tasks;
                collectNodes(this, tasks);
                for (auto node : tasks)
                {
                    if (auto task = dynamic_cast<TaskNode*>(node))
                        task->addObserveAbortCondition(decorator.get());
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  시퀀스를 실행한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
EBTNodeResult SequenceNode::execute(Npc& npc, BTRunningTaskData& runningTaskData) const
{
    for (const auto& decorator : _decorators) {
        if (!decorator->checkCondition(npc)) {
            return EBTNodeResult::Failed;
        }
    }

    auto start = runningTaskData.getCompositeChildIndex(this);
    for (int32_t i = start; i < _children.size(); ++i) {
        auto& child = _children[i];
        auto result = child->execute(npc, runningTaskData);

        if (result == EBTNodeResult::InProgress) {
            runningTaskData.setCompositChildIndex(this, i);
            return EBTNodeResult::InProgress;
        }
        else if (result == EBTNodeResult::Failed) {
            return EBTNodeResult::Failed;
        }
    }
    
    return EBTNodeResult::Succeeded;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  셀렉터를 실행한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
EBTNodeResult SelectorNode::execute(Npc& npc, BTRunningTaskData& runningTaskData) const
{
    for (const auto& decorator : _decorators) {
        if (!decorator->checkCondition(npc)) {
            return EBTNodeResult::Failed;
        }
    }

    auto start = runningTaskData.getCompositeChildIndex(this);

    for (int32_t i = start; i < _children.size(); ++i) {
        auto& child = _children[i];
        auto result = child->execute(npc, runningTaskData);

        if (result == EBTNodeResult::Succeeded) {
            return EBTNodeResult::Succeeded;
        }
        else if (result == EBTNodeResult::InProgress) {
            runningTaskData.setCompositChildIndex(this, i);
            return EBTNodeResult::InProgress;
        }
    }

    return EBTNodeResult::Failed;
}
