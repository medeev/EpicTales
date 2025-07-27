////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		컴포넌트 클래스
///
///	@ date		2024-5-9
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Protocol/Struct/PktTypes.h>

#include "Component/IComponent.h"

class User;
class CloseComponent
	:
	public IComponent 
{
	typedef IComponent super;
	friend class Player;

	const int32_t kCloseClearCheckSec = 60 * 2;
protected:
	Player& _player; ///< 플레이어
	int64_t _closedTimeSec;///< 커넥션이 끊긴시간
	PktObjId _clsoedUserId; ///< 유저식별자
public:
	/// 생성자
	CloseComponent(Player& actor);

	/// 소멸자
	virtual ~CloseComponent();

	/// 유저소유권이 없어질때 처리한다.
	void unPossess(const User& user);

	/// 유저소유권이 생길때 처리한다.
	void possess();

	

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// 정리 한다
	virtual void finalize() override;

	/// 업데이트한다.
	virtual void update(int64_t curTime) override;
};
