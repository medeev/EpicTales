

#include "Pch.h"
#include "CharacterComponent.h"

#include <Data/Info/CharacterInfo.h>
#include <Data/Info/WorldInfo.h>
#include <Protocol/PktCommon.h>
#include <Protocol/Struct/PktGrowth.h>
#include <Protocol/Struct/PktPlayer.h>
#include <Protocol/Struct/PktShape.h>

#include "Actor/Player.h"
#include "DB/CacheTx.h"
#include "OrmSub/PlayerCharacter.h"
#include "OrmSub/CharacterDeck.h"
#include "Room/World.h"
#include "Data/Info/InfoEnumsConv.h"
#include "Util/IdFactory.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CharacterComponent::CharacterComponent(Player& player)
	:
	super(EComponentType::Character, player),
	_player(player), _expInfo(nullptr)
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
CharacterComponent::~CharacterComponent()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		초기화한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterComponent::initialize()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		정리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterComponent::finalize()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	DB객체로 초기화한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CharacterComponent::initializeDB(const DataBaseTarget& db)
{
	auto ormCharacters = DBOrm::PlayerCharacter::SelectListByOwnerId(db, _player.getId());
	for (const auto& characterOrm : ormCharacters)
	{
		CharacterInfoPtr characterInfo(characterOrm->getInfoId());
		ENSURE(characterInfo, continue);

		auto character = std::make_shared<PlayerCharacter>(*characterInfo);
		characterOrm->copyMember(*character);

		_characters.insert(std::make_pair(character->getId(), character));

		
	}

	

	auto ormSlots = DBOrm::CharacterDeck::SelectListByOwnerId(
		db, _player.getId());

	for (auto orm : ormSlots)
	{
		auto characterDeck = std::make_shared<CharacterDeck>();
		orm->copyMember(*characterDeck);

		insertCharacterSlot(characterDeck);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  DB초기화가 모두 다 끝나고 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterComponent::initializeDBPost(const DataBaseTarget& db)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  종료업데이트를 처리한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterComponent::updateDisconnect(CacheTx& cacheTx)
{
	auto taskManager = _player.getTaskManager();


}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  내보낸다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterComponent::exportTo(PktCurrencies& pktCurrencies) const
{
	_currencies.exportTo(pktCurrencies);

	
}

void CharacterComponent::exportTo(PktPlayerCharacters& pktPlayerCharacters) const
{
	for (const auto& character : _characters | std::views::values)
	{
		PktPlayerCharacter pktPlayerCharacter;
		character->exportTo(pktPlayerCharacter);

		pktPlayerCharacters.emplace_back(std::move(pktPlayerCharacter));
	}
}

void CharacterComponent::exportTo(std::vector<PktCharacterSlot>& dst) const
{
	auto iter = _decks.find(kDeckId);
	if (iter == _decks.end())
		return;

	if (auto deck = iter->second)
	{
		std::map<ECharacterSlot, uint64_t> slotMap;
		deck->exportTo(slotMap);

		for (const auto& [slot, characterId] : slotMap)
		{
			PktCharacterSlot pktSlot(slot, characterId);
			dst.emplace_back(std::move(pktSlot));
		}
	}
}

void CharacterComponent::exportTo(std::map<ECharacterSlot, uint64_t>& dst) const
{
	auto iter = _decks.find(kDeckId);
	if (iter == _decks.end())
		return;

	if (auto deck = iter->second)
		deck->exportTo(dst);
}

void CharacterComponent::exportTo(uint64_t characterId, PktStatPointDistribute& pktStatDistribute) const
{
	auto iter = _characters.find(characterId);
	if (iter == _characters.end())
		return;

	auto character = iter->second;
	character->exportTo(pktStatDistribute);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  재화를 변경한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterComponent::change(
	ECurrencyType type, int32_t value)
{
	auto cur = _currencies.get(type);
	auto next = cur + value;

	_currencies.set(type, next);

	PktCurrencyChangeNotify notify;
	notify.getCurrencies().emplace_back(type, next - cur, next);
	_player.send(notify);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  
////////////////////////////////////////////////////////////////////////////////////////////////////
EResultCode CharacterComponent::change(CacheTx& cacheTx, uint64_t characterId, const PktStatPointDistribute& src) const
{
	auto iter = _characters.find(characterId);
	if (iter == _characters.end())
		return EResultCode::InvalidActorId;

	auto character = iter->second;

	auto cacheCharacter = cacheTx.acquireObject(_player, character);
	cacheCharacter->setDex(src.getDex());
	cacheCharacter->setIntellect(src.getIntellect());
	cacheCharacter->setStr(src.getStr());
	cacheCharacter->setRemain(src.getRemain());
	cacheCharacter->updateCache();

	return EResultCode::Success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  경험치를 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
void CharacterComponent::increaseExp(CacheTx& cacheTx, uint64_t characterId, int32_t value)
{
	if (!value)
		return;

	auto iter = _characters.find(characterId);
	if (iter == _characters.end())
		return;

	auto character = iter->second;
	auto oldExp = character->getExp();

	auto cacheCharacter = cacheTx.acquireObject(_player, character);
	auto nextExp = Core::Numeric::Max((int64_t)0, cacheCharacter->getExp() + value);
	cacheCharacter->updateCache();


	{
		PktPlayerChangeExpNotify notify;
		notify.setCharacterId(characterId);
		notify.setExp(nextExp);
		notify.setChange((int32_t)(nextExp - oldExp));

		_player.send(notify);
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  신규 케릭터를 DB에추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CharacterComponent::insertCharacter(
	CacheTx& cacheTx, PktInfoId infoId, PktPlayerCharacter& pktDest) const
{
	CharacterInfoPtr characterInfo(infoId);
	if (!characterInfo)
	{
		WARN_LOG("not exist characterInfo [pid:%llu, infoId:%u", _player.getId(), infoId);
		return false;
	}

	auto newCharacter = std::make_shared<PlayerCharacter>(*characterInfo);
	newCharacter->setId(Core::IdFactory::CreateId());
	newCharacter->setOwnerId(_player.getId());
	newCharacter->setInfoId(characterInfo->getId());
	auto cacheItem = cacheTx.acquireObject(_player, newCharacter);
	cacheItem->insertCache();

	newCharacter->exportTo(pktDest);

	return true;
}

void CharacterComponent::insertCharacter(PlayerCharacterPtr character)
{
	ENSURE(character, return);

	_characters.insert(std::make_pair(character->getId(), character));
}

const PlayerCharacterPtr CharacterComponent::findPlayerCharacter(uint64_t id) const
{
	auto iter = _characters.find(id);
	if (iter == _characters.end())
		return nullptr;

	return iter->second;
}

PlayerCharacterPtr CharacterComponent::findPlayerCharacter(uint64_t id)
{
	return std::const_pointer_cast<PlayerCharacter>(static_cast<const CharacterComponent&>(*this).findPlayerCharacter(id));
}

EResultCode CharacterComponent::changeSlot(CacheTx& cacheTx, ECharacterSlot slot, uint64_t characterId) const
{
	auto iter = _decks.find(kDeckId);
	if (iter == _decks.end())
		return EResultCode::LogicError;

	auto deck = iter->second;
	if (!deck)
		return EResultCode::LogicError;

	auto cacheDeck = cacheTx.acquireObject(_player, deck);
	cacheDeck->setCharacterSlot(slot, characterId);
	cacheDeck->updateCache();

	return EResultCode::Success;
}

bool CharacterComponent::insertCharacterSlot(CharacterDeckPtr characterDeck)
{
	ENSURE(characterDeck, return false);

	auto ret = _decks.insert(std::make_pair(characterDeck->getDeckId(), characterDeck));
	if (!ret.second)
	{
		WARN_LOG("slot is duplcated. [ownerId:%llu, deckId:%u]",
			characterDeck->getOwnerId(), characterDeck->getDeckId());
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  
////////////////////////////////////////////////////////////////////////////////////////////////////
std::list<ECharacterSlot> CharacterComponent::findEmptySlots()
{
	std::list<ECharacterSlot> emptySlots;
	auto iter = _decks.find(kDeckId);
	if (iter == _decks.end())
		return emptySlots;

	auto deck = iter->second;

	for (const auto& type : getEnums<ECharacterSlot>())
	{
		if (type == ECharacterSlot::Max)
			continue;

		if (deck->getCharacterSlot(type) == 0)
			emptySlots.push_back(type);
	}

	return emptySlots;
}
