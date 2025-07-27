////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief        BTNode
///
/// @date         2024-7-30
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "BTNodeTypes.h"

class BTNode;
class TaskNode;
class CompositeNode;
class BTRunningTaskData;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  특정 노드의 모든 오른쪽 노드를 반환하는 함수
////////////////////////////////////////////////////////////////////////////////////////////////////
void collectNodes(BTNode* root, std::vector<BTNode*>& result);
std::vector<BTNode*> getRightNodes(BTNode* target);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  기본 노드입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
class Npc;
class BTNode
{
    friend class CompositeNode;
    friend class TaskNode;
public:
    BTNode() : _parent(nullptr) {}
    virtual EBTNodeResult execute(Npc& npc, BTRunningTaskData& runningTaskData) const = 0;

    /// If처리하는 Decorator에만 적용하도록 하여 하위 Or 하위 Sibling의 모든 Task가 현재 Deco를 감시하게 한다.
    virtual void buildObserveAbort() = 0;

    void setParent(BTNode* parent) {
        _parent = parent;
    }
    BTNode* getParent() const {
        return _parent;
    }
    void setNodeName(const std::string& nodeName) {
        _nodeName = nodeName;
    }
    const std::string& getNodeName() const {
        return _nodeName;
    }

private:
    BTNode* _parent;
    std::string _nodeName;
};

using BTNodePtr = std::shared_ptr<BTNode>;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  체크노드입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
class DecoratorNode 
{
public:
    enum class EDecoType
    {
        kIfCheck, ///< If문처럼 작동하는 데코레이터
        kNodeSetting, ///< 노드에 먼가를 설정하는 데코레이터
    };

    DecoratorNode(
        EDecoType type,
        std::function<bool(const BTNode&, Npc&)> condition,
        EBTFlowAbortMode abortType = EBTFlowAbortMode::None)
        : _type(type), _condition(condition), _abortType(abortType), _owner(nullptr)
    {
    }
    void setNodeName(const std::string& nodeName) {
        _nodeName = nodeName;
    }

    bool checkCondition(Npc& npc) const {
        return _condition(*_owner, npc) != _inverse;
    }

    void setOwner(BTNode* owner) {
        _owner = owner;
    }

    const EDecoType getDecoType() const {
        return _type;
    }

    const EBTFlowAbortMode getAbortType() const {
        return _abortType;
    }

    void setInverse(bool inverse) {
        _inverse = inverse;
    }
    bool isInverse() const {
        return _inverse;
    }

private:
    BTNode* _owner;
    EDecoType _type;
    std::function<bool(const BTNode&, Npc&)> _condition;
    EBTFlowAbortMode _abortType;
    bool _inverse;
    std::string _nodeName;
};

using DecoratorNodePtr = std::shared_ptr<DecoratorNode>;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Task노드입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
class TaskNode : public BTNode
{
    friend class BTNode;
public:
    struct TaskCheckCommonData 
    {
        TaskCheckCommonData() :
            excuteLimit(0), limitReturn(false), coolTimeMSec(0) {}

        int32_t excuteLimit;
        bool limitReturn;
        int32_t coolTimeMSec;
    };

    TaskNode(std::function<EBTNodeResult(const TaskNode& selfTask, Npc&)> task)
        : _task(task) {}

    void addDecorator(DecoratorNodePtr decorator) {
        if (!decorator)
            return;
        decorator->setOwner(this);
        _decorators.push_back(decorator);
    }
    void addObserveAbortCondition(DecoratorNode* deco)
    {
        _decoratorObserves.insert(deco);
    }

    virtual EBTNodeResult execute(Npc& npc, BTRunningTaskData& runningTaskData) const override;
    virtual void buildObserveAbort() override;

    const TaskCheckCommonData& getCheckCommonData() const {
        return _taskCheckCommonData;
    }
    TaskCheckCommonData& getCheckCommonData() {
        return _taskCheckCommonData;
    }

    void print(Npc& npc, EBTNodeResult result) const;
private:

    std::function<EBTNodeResult(const TaskNode&, Npc&)> _task;
    std::vector<DecoratorNodePtr> _decorators;
    std::set<DecoratorNode*> _decoratorObserves;
    TaskCheckCommonData _taskCheckCommonData;
   
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  컴포짓노드입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
class CompositeNode : public BTNode
{
public:
    void addChild(BTNodePtr child) {
        if (!child)
            return;
        child->setParent(this);
        _children.push_back(child);
    }

    void addDecorator(DecoratorNodePtr decorator) {
        if (!decorator)
            return;
        decorator->setOwner(this);
        _decorators.push_back(decorator);
    }
    const std::vector<BTNodePtr>& getChildren() const {
        return _children;
    }

    virtual void buildObserveAbort() override;


protected:
    std::vector<BTNodePtr> _children;
    std::vector<DecoratorNodePtr> _decorators;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  시퀀스 노드입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
class SequenceNode : public CompositeNode 
{
public:
    virtual EBTNodeResult execute(Npc& npc, BTRunningTaskData& runningTaskData) const override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  셀렉트 노드입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////
class SelectorNode : public CompositeNode 
{
public:
    virtual EBTNodeResult execute(Npc& npc, BTRunningTaskData& runningTaskData) const override;
};
