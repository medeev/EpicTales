﻿////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		FSM State초기화
///
///	@ date		2024-12-10
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  상태초기화
////////////////////////////////////////////////////////////////////////////////////////////////////
class StateInit
{
public:
    virtual ~StateInit() {}

    template<typename T>
    const T* cast() const {
        return dynamic_cast<const T*>(this);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc대기상태초기화
////////////////////////////////////////////////////////////////////////////////////////////////////
class NpcStateInitWait : public StateInit
{
public:
    int64_t endTime;

public:
    NpcStateInitWait() : endTime(0) {}
};



////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc스킬상태초기화
////////////////////////////////////////////////////////////////////////////////////////////////////
class NpcStateInitSkill : public StateInit
{
public:
	const SkillInfo& _skillInfo;
	ActorPtr _target;

public:
	NpcStateInitSkill() = delete;
	NpcStateInitSkill(const SkillInfo& info, ActorPtr target) : _skillInfo(info), _target(target) {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Npc위치이동 상태초기화
////////////////////////////////////////////////////////////////////////////////////////////////////
class NpcStateInitMoveTo : public StateInit
{
public:
	const Vector _location;
	const int32_t _range;

public:
	NpcStateInitMoveTo() = delete;
	NpcStateInitMoveTo(const Vector& location, int32_t range) : _location(location), _range(range) {}
};


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  플레이어스킬상태초기화
////////////////////////////////////////////////////////////////////////////////////////////////////
class PlayerStateInitSkill : public StateInit
{
public:
	const SkillInfo& _skillInfo;
	ActorPtr _target;

public:
	PlayerStateInitSkill() = delete;
	PlayerStateInitSkill(const SkillInfo& info, ActorPtr target) : _skillInfo(info), _target(target) {}
};
