////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		룸스레드
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Core/Task/TaskManager.h>
#include <Core/Thread/IThread.h>

class Party;
typedef std::shared_ptr<Party> PartyPtr;

class Matching;
typedef std::shared_ptr<Matching> MatchingPtr;

class PartyManager
	:
	public Core::Singleton<PartyManager>, 
	public Core::IThread
{
	friend class Core::Singleton<PartyManager>;
	
	typedef std::unordered_map< uint64_t, PartyPtr > PartyMap; ///< 옥션맵 타입 정의
	typedef std::unordered_map< uint64_t, MatchingPtr > MatchingMap; ///< 매칭맵 타입 정의

public:
	/// 콜백 타입 정의
	typedef std::function< void(PartyPtr) > Callback;

private:
	PartyMap _parties; ///< 옥션목록
	MatchingMap _matchings; ///< 매칭목록
	Core::TaskManager _taskManager; ///< 작업관리자

private:
	/// 생성자
	PartyManager();

	/// 소멸자
	~PartyManager();

public:

	/// 순회한다
	void foreachParty(const Callback& callback);

	/// 해당 스레드에서 작업을 실행한다
	void runTask(const Core::TaskCaller& taskCaller, const Core::TaskManager::Task& task);

	/// 옥션을 추가한다
	void addParty(PartyPtr room);

	/// 옥션를 가져온다.
	PartyPtr getParty(uint64_t id);

	/// 옥션를 가져온다.
	const PartyPtr getParty(uint64_t id) const;

public:
	/// 스레드 코드를 실행한다
	virtual void onRun() override;
};
