

#pragma once

#include <stdint.h>

class Actor;
typedef std::shared_ptr<Actor> ActorPtr;
typedef std::weak_ptr<Actor> ActorWeakPtr;
typedef std::unordered_map<uint64_t, ActorPtr> ActorMap;
typedef std::unordered_map<uint64_t, ActorWeakPtr> ActorWeakMap;
typedef std::shared_ptr<ActorMap> ActorMapPtr;
typedef std::vector<ActorPtr> Actors;

class Player;
typedef std::shared_ptr<Player> PlayerPtr;
typedef std::weak_ptr<Player> PlayerWeakPtr;
typedef std::unordered_map<uint64_t, PlayerPtr> PlayerMap;
typedef std::unordered_map<uint64_t, PlayerWeakPtr> PlayerWeakMap;

class Npc;
typedef std::shared_ptr<Npc> NpcPtr;
typedef std::weak_ptr<Npc> NpcWeakPtr;
typedef std::unordered_map<uint64_t, NpcPtr> NpcMap;

class EnvObj;
typedef std::shared_ptr<EnvObj> EnvObjPtr;
typedef std::weak_ptr<EnvObj> EnvObjWeakPtr;

class Projectile;
typedef std::shared_ptr<Projectile> ProjectilePtr;
typedef std::list<ProjectilePtr> ProjectileList;

class PickupItem;
typedef std::shared_ptr<PickupItem> PickupItemPtr;
typedef std::weak_ptr<PickupItem> PickupItemWeakPtr;



/// 플레이어 콜백 타입 정의
typedef std::function<void(const PlayerPtr&)> PlayerCallback;

/// 엔피시 콜백 타입 정의
typedef std::function<void(const NpcPtr&)> NpcCallback;

/// 액터 콜백 타입 정의
typedef std::function<void(const ActorPtr&)> ActorCallback;


