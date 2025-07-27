////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		스폰관리자타입 목록
///
///	@ date		2024-3-15
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

class NpcSpawner;

/// NPC 스폰 관리자 포인터 타입 정의
typedef std::shared_ptr< NpcSpawner > NpcSpawnerPtr;

/// NPC 스폰 관리자 위크 포인터 타입 정의
typedef std::weak_ptr< NpcSpawner > NpcSpawnerWeakPtr;


class EnvObjSpawner;

/// EnvObj 스폰 관리자 포인터 타입 정의
typedef std::shared_ptr< EnvObjSpawner > EnvObjSpawnerPtr;

/// EnvObj 스폰 관리자 위크 포인터 타입 정의
typedef std::weak_ptr< EnvObjSpawner > EnvObjSpawnerWeakPtr;

