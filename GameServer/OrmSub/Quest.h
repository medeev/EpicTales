////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		OrmQuest 상속 클래스
///
///	@ date		2024-4-26
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <DB/Orms/OrmQuest.h>

class PktQuest;
class QuestInfo;
class Quest : public DBOrm::Quest
{
	Quest() = delete;
protected:
	const QuestInfo& _questInfo;
public:
	/// 생성자
	Quest(const QuestInfo& info) : _questInfo(info) {}

	/// 커스텀 캐시에 반영한다.
	virtual void applyCache(DBOrm::OrmObjectPtr origin) const override;

	const QuestInfo& getInfo() const {
		return _questInfo;
	}

	std::shared_ptr<Quest> clone() const;

	/// 퀘스트 정보를 내보낸다.
	void exportTo(PktQuest& pktQuest) const;

	/// 테스크 횟수를 설정한다.
	void setTaskCount(int32_t index, int32_t count);

	/// 테스크 횟수를 가져온다.
	int32_t getTaskCount(int32_t index) const;
};

