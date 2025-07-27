////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		송신 스레드
///
///	@ date		2024-3-7
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Core/Task/TaskManager.h>
#include <Core/Thread/IThread.h>

class SendThread
	:
	public Core::IThread
{
private:
	Core::TaskManager _taskManager;	///< 테스크 관리자

public:
	/// 생성자
	SendThread();

	/// 테스크 관리자를 반환한다
	Core::TaskManager& getTaskManager() { 
		return _taskManager; }

	/// 정지시 처리한다
	virtual void stop() override;

private:
	/// 실행한다
	virtual void onRun() override;
};