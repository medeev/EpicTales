////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		OrmMail 상속 클래스
///
///	@ date		2024-4-26
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms/OrmMail.h>

class PktMail;
class MailInfo;
class Mail : public DBOrm::Mail
{
public:
	/// 생성자
	Mail(){}

	/// 커스텀 캐시에 반영한다.
	virtual void applyCache(DBOrm::OrmObjectPtr origin) const override;

	/// 객체를 클론한다
	std::shared_ptr<Mail> clone() const;

	/// 메일 정보를 내보낸다.
	void exportTo(PktMail& pktMail) const;
};

