////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		월드 관리자
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Core/Thread/SharedMutex.h>

#include "World.h"

class RoomThread;

class WorldManager
	:
	public Core::Singleton<WorldManager>
{
	/// 채널식별자 타입정의
	typedef int32_t ChannelId;

	/// 룸 맵 타입 정의
	typedef std::map<uint64_t, WorldPtr> Worlds;

	/// 생성함수
	typedef std::function<WorldPtr(RoomThread&)> CreateFunction;

	/// 콜백 타입 정의
	typedef std::function<void(WorldPtr)> Callback;

	/// 채널 별 룸 맵 타입 정의
	typedef std::map<ChannelId, WorldPtr> Channels;

	/// 공개 월드 맵 타입 정의
	typedef std::map<int32_t, Channels> PublicChannels;
private:
	Worlds            _worlds;     ///< 월드 목록
	Core::SharedMutex _worldLock;  ///< 월드 목록 락
	PublicChannels    _publicChannels; ///< 공개 채널 목록


public:
	/// 초기화 한다
	void initialize();

	/// 해제한다.
	void finailze();

	/// 월드을 획득한다
	WorldPtr acquireWorld(
		int32_t worldInfoId, 
		bool isNew,
		const CreateFunction& createFunc);

	/// 월드를 삭제한다
	void deleteWorld(PktObjId id);

	/// 월드를 반환한다
	WorldPtr getWorld(PktObjId worldId);

	/// 모든 룸의 액터 카운트 정보를 출력한다.
	void printInfo();


private:
};
