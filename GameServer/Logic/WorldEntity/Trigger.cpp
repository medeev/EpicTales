
#include "Pch.h"
#include "Trigger.h"

#include "Data/Info/WorldInfo.h"
#include "Room/World.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
Trigger::Trigger(World& world) : _world(world), _cellNo(-1)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  업데이트 한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void Trigger::update()
{
    if (!_collider)
        return;

    if (_cellNo == -1)
        return;

    Actors outActor;
    findOverlapActors(outActor);
    
    std::set<ActorPtr> nexts;
    for (const auto& actor : outActor)
    {
        if (actor->getWorld() != &_world)
            continue;

        nexts.insert(actor);
    }

    std::set<ActorPtr> leaves;
    std::set_difference(_inActors.begin(), _inActors.end(), nexts.begin(), nexts.end(),
        std::inserter(leaves, leaves.begin()));

    std::set<ActorPtr> enters;
    std::set_difference(nexts.begin(), nexts.end(), _inActors.begin(), _inActors.end(),
        std::inserter(enters, enters.begin()));

    _inActors = nexts;

    for (auto enter : enters)
        enter->broadcastTriggerEnter(*this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  안에 있는 액터를 조사한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Trigger::findOverlapActors(Actors& actors) const
{
    return _world.getCollisionOverlapActors(_cellNo, _collider, actors, [](const ActorPtr& actor) {return true; });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  
////////////////////////////////////////////////////////////////////////////////////////////////////
void TriggerBox::init(const TriggerBoxData& data)
{
    Core::Collision::BoxPtr box = std::make_shared<Core::Collision::Box>();
    box->init(data.getExtern());
    _collider = box;
    _collider->setPosition(data.getLocation());
    _collider->setRotate(Core::Rotator(0, data.getRotator().getYaw(), 0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  
////////////////////////////////////////////////////////////////////////////////////////////////////
void TriggerSphere::init(const TriggerSphereData& data)
{
    auto sphere = std::make_shared<Core::Collision::Sphere>();
    sphere->init(data.getRadius());
    _collider = sphere;
    _collider->setPosition(data.getLocation());
    _collider->setRotate(Core::Rotator(0, data.getRotator().getYaw(), 0));
}
