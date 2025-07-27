////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		컴포넌트 클래스
///
///	@ date		2024-5-9
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Protocol/Struct/PktTypes.h>

#include "Actor/ActorTypes.h"
#include "Component/IComponent.h"
#include "DB/CacheTx.h"
#include "Logic/Counts/ActorStat.h"
#include "Logic/Counts/Currencies.h"
#include "OrmSub/OrmSubType.h"

class CharacterInfo;
class ExpInfo;
class EffectApply;
class Player;
class PktPlayerCharacter;
class PktStatPointDistribute;
class CharacterComponent
	:
	public IComponent
{
	typedef IComponent super;

	friend class Player;
	const uint8_t kDeckId = 0;
protected:
	Player& _player; ///< 플레이어
	
	Currencies _currencies;
	const ExpInfo* _expInfo;
	std::map<uint64_t, PlayerCharacterPtr> _characters;
	std::map<uint8_t, CharacterDeckPtr> _decks; ///< 덱Id, 덱정보


public:
	/// 생성자
	CharacterComponent(Player& actor);

	/// 소멸자
	virtual ~CharacterComponent();

private:
	/// 초기화 한다
	virtual void initialize() override;

	/// DB객체로 초기화 한다
	virtual bool initializeDB(const DataBaseTarget& db) override;

	/// DB 초기화 이후에 처리한다.
	virtual void initializeDBPost(const DataBaseTarget& db) override;

	/// 정리 한다
	virtual void finalize() override;

public:
	/// 종료업데이트를 한다.
	void updateDisconnect(CacheTx& cacheTx);

	

	/// 내보낸다.
	void exportTo(PktCurrencies& pktCurrencies) const;
	void exportTo(PktPlayerCharacters& pktPlayerCharacters) const;
	void exportTo(uint64_t characterId, PktStatPointDistribute& pktStatDistribute) const;

	/// 재화를 가져온다.
	const Currencies& getCurrencies() const {
		return _currencies;
	}
	Currencies& getCurrencies() {
		return _currencies;
	}

	/// 재화를 변경한다.
	void change(
		ECurrencyType type,
		int32_t value);

	/// 변경한다
	EResultCode change(
		CacheTx& cacheTx,
		uint64_t characterId,
		const PktStatPointDistribute& src) const;

	/// 경험치를 추가한다.
	void increaseExp(CacheTx& cacheTx, uint64_t characterId, int32_t value);

	

	/// 케릭터를 DB에 추가하고 패킷형태로 반환한다.
	bool insertCharacter(
		CacheTx& cacheTx,
		PktInfoId infoId,
		PktPlayerCharacter& pktDest) const;

	void insertCharacter(PlayerCharacterPtr character);
	const PlayerCharacterPtr findPlayerCharacter(uint64_t id) const;
	PlayerCharacterPtr findPlayerCharacter(uint64_t id);

	/// 슬롯을 변경한다
	EResultCode changeSlot(
		CacheTx& cacheTx, ECharacterSlot slot, uint64_t characterid) const;
	bool insertCharacterSlot(CharacterDeckPtr characterDeck);
	std::list<ECharacterSlot> findEmptySlots();

	void exportTo(std::vector<PktCharacterSlot>& dst) const;
	void exportTo(std::map<ECharacterSlot, uint64_t>& dst) const;

private:

};
