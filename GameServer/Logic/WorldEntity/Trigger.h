#pragma once


#include <Core/Collision/Collider.h>
#include <Core/Container/Delegate.h>

#include "Actor/ActorTypes.h"

class CacheTx;
class Player;
class Npc;
class Actor;
class TriggerBoxData;
class TriggerSphereData;

class World;
class Trigger
{
protected:
    World& _world;
    int32_t _cellNo;
    Core::Collision::ColliderPtr _collider; ///< 가나다
    std::set<ActorPtr> _inActors;

public:
    Trigger(World&);
    virtual ~Trigger() {}

    void update();
    bool findOverlapActors(Actors& actors) const;
};



////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  박스형 트리거
////////////////////////////////////////////////////////////////////////////////////////////////////
class TriggerBox : public Trigger
{
public:
    TriggerBox(World& world) : Trigger(world) {}
    void init(const TriggerBoxData& data);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  구체트리거
////////////////////////////////////////////////////////////////////////////////////////////////////
class TriggerSphere : public Trigger
{
public:
    TriggerSphere(World& world) : Trigger(world) {}
    void init(const TriggerSphereData& data);
};
