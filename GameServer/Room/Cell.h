////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		셀 객체	
///
///	@ date		2024-3-13
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <unordered_map>
#include <unordered_set>

#include "Actor/ActorTypes.h"

class Npc;
class Player;


/// 셀 번호 목록 타입 정의
typedef std::vector<int32_t> CellNos;

/// 셀 번호 목록 타입 정의
typedef std::set<int32_t> CellNoSet;

class Cell
{
private:
	CellNos   _nearCells;   ///< 인접 셀 목록
	CellNoSet _nearCellSet; ///< 인접 셀 목록
	int32_t   _cellNo;      ///< 셀 번호

	int32_t   _width_i;     ///< 왼쪽 좌표
	int32_t   _height_i;     ///< 위쪽 좌표

	ActorMap  _actors;      ///< 액터 목록
	PlayerMap _players;     ///< 플레이어 목록
	NpcMap    _npcs;        ///< 엔피시 목록

public:
	/// 생성자
	Cell();

	/// 소멸자
	~Cell();

	/// 초기화 한다
	bool initialize(
		int32_t cellNo,
		int32_t widthCount,
		int32_t heightCount,
		int32_t left,
		int32_t top);

	/// 액터를 추가한다
	void addActor(Actor& actor);

	/// 액터를 제거한다
	void removeActor(uint64_t actorId);

	/// 인접 셀 리스트를 반환한다
	const CellNos& getNearCellList() const {
		return _nearCells;
	}

	/// 인접 셀인지 여부를 반환한다
	const bool isNearCellNo(int32_t cellNo) const {
		return _nearCellSet.find(cellNo) != _nearCellSet.end();
	}

	/// 셀 LeftTop을 얻어온다.
	Vector getLeftTop() const {
		return Vector((float)_width_i, (float)_height_i, 0.f);
	}

	/// Actor를 순횐한다
	void foreachActor(const ActorCallback& callback) const;

	/// Npc를 순회한다.
	void foreachNpc(const NpcCallback& callback) const;

	/// Player를 순회한다.
	void foreachPlayer(const PlayerCallback& callback) const;

private:
	/// 인접셀을 계산한다
	void _calcNearCells(int32_t widthCount, int32_t heightCount);

	/// 액터를 추가한다
	void _addActor(Actor& actor);

	/// 액터를 제거한다
	void _removeActor(uint64_t actorId);
};
