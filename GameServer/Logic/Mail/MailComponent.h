////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		컴포넌트 클래스
///
///	@ date		2024-5-9
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Protocol/Struct/PktTypes.h>

#include "Actor/ActorTypes.h"
#include "Component/IComponent.h"
#include "DB/CacheTx.h"
#include "OrmSub/OrmSubType.h"

class PktMail;
class Player;
class MailComponent
	:
	public IComponent 
{
	typedef IComponent super;
protected:
	Player& _player; ///< 플레이어
	std::map<PktObjId, MailPtr> _mails;

public:
	/// 생성자
	MailComponent(Player& actor);

	/// 소멸자
	virtual ~MailComponent();

	/// 메일을 추가한다.
	void insertMail(MailPtr mail);

	/// 패킷으로 내보낸다.
	void exportTo(std::vector<PktMail>& outMails) const;

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// DB객체로 초기화 한다
	virtual bool initializeDB(const DataBaseTarget& db) override;

	/// 정리 한다
	virtual void finalize() override;
};
