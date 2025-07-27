////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		셀 관리자
///
///	@ date		2024-3-13
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once


#include <Core/Util/Rect.h>

#include "Actor/ActorTypes.h"
#include "Cell.h"

class Room;

class CellManager
{
public:
	/// 셀 목록 타입 정의
	typedef std::vector<Cell> Cells;

private:
	Room&    _room;		   ///< 룸
	Cells _cells;	   ///< 셀 목록
	int32_t	 _widthCount;	   ///< 행 갯수
	int32_t	 _heightCount;	   ///< 열 갯수
	int32_t	 _oneCellSize; ///< 한 셀 사이즈
	int32_t	 _left;		   ///< 왼쪽끝 좌표
	int32_t	 _top;		   ///< 위쪽끝 좌표
	int32_t	 _right;	   ///< 오른쪽끝 좌표
	int32_t	 _bottom;	   ///< 아래쪽끝 좌표

public:
	/// 생성자
	CellManager(Room& room);

	/// 소멸자
	virtual ~CellManager();

	/// 초기화 한다
	bool initialize(int32_t oneCellSize, const Core::Rect<float>& rect);

	/// 액터를 추가한다
	void addActor(Actor&);

	/// 액터를 제거한다
	void removeActor(Actor* actor);

	/// 좌표에 해당하는 셀 번호를 반환한다
	int32_t getCellNo(const Vector& pos) const;

	/// 플레이어를 순횐한다
	void foreachPlayer(const int32_t cellNo, const PlayerCallback& callback) const;

	/// 플레이어를 순횐한다
	void foreachPlayer(const CellNos& cellNoList, const PlayerCallback& callback) const;

	/// 액터를 순횐한다
	void foreachActor(const int32_t cellNo, const ActorCallback& callback) const;

	/// 액터를 순횐한다
	void foreachActor(const CellNos& cellNoList, const ActorCallback& callback) const;

	/// 엔피시를 순횐한다
	void foreachNpc(const int32_t cellNo, const NpcCallback& callback) const;

	/// 엔피시를 순횐한다
	void foreachNpc(const CellNos& cellNoList, const NpcCallback& callback) const;

	/// 액터가 움직일때 처리한다.
	bool onCellChange(Actor& actor);

	/// 원셀사이즈를 반환한다.
	int32_t getOneCellSize() const {
		return _oneCellSize;
	}

	/// 대상이 소스의 인접Cell인지 반환한다.
	bool isNearCellNo(int32_t src, int32_t dest) const;

	/// 셀의 LeftTop을 가져온다
	bool getCellLeftTop(int32_t cellNo, Vector& dest) const;

private:
	/// 셀 바운드를 체크한다
	bool _checkCellBound(int32_t cellNo) const;

	/// 셀을 변경한다
	bool _cellChange(Actor& actor, int32_t oldCellNo, int32_t newCellNo);

	/// 인접 셀을 구한다
	void _nearCellList(int32_t cellNo1, int32_t cellNo2, CellNos& dest);
};
