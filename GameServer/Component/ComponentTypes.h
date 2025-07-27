
#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		컴포넌트 타입
////////////////////////////////////////////////////////////////////////////////////////////////////
enum class EComponentType : uint8_t
{
	Character = 0,
	Aggro,
	Attentioned,
	Closer,
	CutScene,
	Dungeon,
	EffectBuff,
	Effect,
	Fsm,
	EnvObjSpawn,
	DistributeStat,
	Ignore,
	Inven,
	Equip,
	Mail,
	Achievement,
	Pickup,
	NpcAi,
	NpcMovement,
	NpcReward,
	NpcSpawn,
	ProjectileMovement,
	Quest,
	Skill,
	Trigger,
	Currency,
	Close,
	Max,
};

enum class EComponentUpdate
{
	kNo,
	kYes
};