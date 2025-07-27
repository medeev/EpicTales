

#include "Pch.h"
#include "CellManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CellManager::CellManager(Room& room)
	:
	_room(room), _oneCellSize(0), _widthCount(0), _heightCount(0), _left(0), _top(0), _right(0), _bottom(0)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////
CellManager::~CellManager()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		초기화한다
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CellManager::initialize(int32_t oneCellSize, const Core::Rect<float>& rect)
{
	if (oneCellSize <= 0)
		return false;

	_oneCellSize = oneCellSize;

	_left = (int32_t)(rect.left) - _oneCellSize;
	_top = (int32_t)(rect.top) - _oneCellSize;
	_right = (int32_t)(rect.right) + _oneCellSize;
	_bottom = (int32_t)(rect.bottom) + _oneCellSize;

	const int32_t width = _right - _left;
	if (width <= 0)
		return false;

	const int32_t height = _bottom - _top;
	if (height <= 0)
		return false;

	_widthCount = width / _oneCellSize + (_oneCellSize < width ? 1 : 0);
	_heightCount = height / _oneCellSize + (_oneCellSize < height ? 1 : 0);
	const int32_t cellCount = _widthCount * _heightCount;

	_cells.reserve(cellCount);
	
	for (int32_t width_i = 0; width_i < _widthCount; ++width_i)
	{
		for (int32_t height_i = 0; height_i < _heightCount; ++height_i)
		{
			auto cellNo = _heightCount * width_i + height_i;

			Cell cell;
			cell.initialize(cellNo, _widthCount, _heightCount, width_i, height_i);
			_cells.emplace_back(std::move(cell));
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		액터를 추가한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CellManager::addActor(Actor& actor)
{
	int32_t cellNo = getCellNo(actor.getLocation());

	if (_checkCellBound(cellNo))
	{
		_cells[cellNo].addActor(actor);
		actor.setCellNo(cellNo);
	}
	else
	{
		WARN_LOG("failed to cell bound [actorId: %lld], complusion set CellNo is invalid!!!!", actor.getId());
	}
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		액터를 제거한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CellManager::removeActor(Actor* actor)
{
	ENSURE(actor, return);

	const int32_t cellNo = actor->getCellNo();
	if (_checkCellBound(cellNo))
	{
		_cells[cellNo].removeActor(actor->getId());
		actor->setCellNo(-1);
	}
	else
	{
		WARN_LOG("failed to cell bound [actorId: %lld], complusion set CellNo is invalid!!!!", actor->getId());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		좌표에 해당하는 셀번호를 반환한다
////////////////////////////////////////////////////////////////////////////////////////////////////
int32_t CellManager::getCellNo(const Vector& pos) const
{
	const int32_t width_i = ((int32_t)(pos.x) - _left) / _oneCellSize;
	const int32_t height_i = ((int32_t)(pos.y) - _top) / _oneCellSize;

	return _heightCount * width_i + height_i;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		플레이어를 순회한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CellManager::foreachPlayer(const int32_t cellNo, const PlayerCallback& callback) const
{
	if (cellNo < 0 || cellNo >= (int32_t)(_cells.size()))
	{
		WARN_LOG("invalid cellNo [cellNo: %d]", cellNo);
		return;
	}

	const Cell& cell = _cells.at(cellNo);

	const auto& cellNoList = cell.getNearCellList();
	foreachPlayer(cellNoList, callback);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		플레이어를 순회한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CellManager::foreachPlayer(const CellNos& cellNoList, const PlayerCallback& callback) const
{
	std::list<PlayerPtr> playerList;
	for (int32_t cellNo : cellNoList)
	{
		if (!_checkCellBound(cellNo))
		{
			WARN_LOG("failed to cell bound [cellNo: %d]", cellNo);
			continue;
		}

		const auto& cell = _cells[cellNo];
		cell.foreachPlayer([&playerList](const PlayerPtr& player)
			{
				playerList.push_back(player);
			});
	}

	for (auto& player : playerList)
		callback(player);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		액터를 순회한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CellManager::foreachActor(const int32_t cellNo, const ActorCallback& callback) const
{
	if (!callback)
		return;

	if (!_checkCellBound(cellNo))
	{
		WARN_LOG("invalid cellNo [cellNo: %d]", cellNo);
		return;
	}

	const auto& cell = _cells.at(cellNo);
	const auto& cellNoList = cell.getNearCellList();
	foreachActor(cellNoList, callback);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		액터를 순회한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CellManager::foreachActor(const CellNos& cellNoList, const ActorCallback& callback) const
{
	if (!callback)
		return;

	std::list<ActorPtr> actorList;
	for (int32_t cellNo : cellNoList)
	{
		if (!_checkCellBound(cellNo))
		{
			WARN_LOG("failed to cell bound [cellNo: %d]", cellNo);
			continue;
		}

		const auto& cell = _cells[cellNo];
		cell.foreachActor([&actorList](const ActorPtr& actor)
			{
				actorList.push_back(actor);
			});
	}

	for (auto& actor : actorList)
		callback(actor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		엔피시를 순회한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CellManager::foreachNpc(const int32_t cellNo, const NpcCallback& callback) const
{
	if (!_checkCellBound(cellNo))
		return;

	const auto& cell = _cells.at(cellNo);
	const auto& cellNoList = cell.getNearCellList();
	foreachNpc(cellNoList, callback);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		엔피시를 순회한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CellManager::foreachNpc(const CellNos& cellNoList, const NpcCallback& callback) const
{
	if (!callback)
		return;

	std::list<NpcPtr> npcList;
	for (int32_t cellNo : cellNoList)
	{
		if (!_checkCellBound(cellNo))
		{
			WARN_LOG("failed to cell bound [cellNo: %d]", cellNo);
			continue;
		}

		const auto& cell = _cells.at(cellNo);
		cell.foreachNpc([&npcList](const NpcPtr& npc)
			{
				npcList.push_back(npc);
			});
	}

	for (auto& npc : npcList)
		callback(npc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	액터가 움직일때 처리한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CellManager::onCellChange(Actor& actor)
{
	int32_t oldCellNo = actor.getCellNo();
	int32_t newCellNo = getCellNo(actor.getLocation());

	if (oldCellNo == newCellNo)
		return true;

	return _cellChange(actor, oldCellNo, newCellNo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  대상이 소스의 인접셀인지 반환한다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CellManager::isNearCellNo(int32_t src, int32_t dest) const
{
	if (!_checkCellBound(src))
		return false;

	if (!_checkCellBound(dest))
		return false;

	const auto& cell = _cells[src];
	return cell.isNearCellNo(dest);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  셀의 LeftTop을 가져온다.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CellManager::getCellLeftTop(int32_t cellNo, Vector& dest) const
{
	if (!_checkCellBound(cellNo))
		return false;

	dest = _cells[cellNo].getLeftTop();
	dest.x = _left + (dest.x * _oneCellSize);
	dest.y = _top + (dest.y * _oneCellSize);


	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		셀바운드를 체크한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CellManager::_checkCellBound(int32_t cellNo) const
{
	if (cellNo < 0 || cellNo >= (int32_t)(_cells.size()))
	{
		WARN_LOG(
			"failed to cell bound [cellNo: %d, cellSize: %zu",
			cellNo,
			_cells.size());
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		셀을 변경한다
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CellManager::_cellChange(Actor& actor, int32_t oldCellNo, int32_t newCellNo)
{
	if (!_checkCellBound(newCellNo))
		return false;

	CellNos cellNoList(18);
	if (_checkCellBound(oldCellNo))
		_nearCellList(oldCellNo, newCellNo, cellNoList);
	else
		cellNoList = _cells[newCellNo].getNearCellList();

	actor.setCellNo(newCellNo);
	actor.updateSight(&cellNoList);

	if (_checkCellBound(oldCellNo))
		_cells[oldCellNo].removeActor(actor.getId());

	_cells[newCellNo].addActor(actor);

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief		인접 셀을 구한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CellManager::_nearCellList(int32_t cellNo1, int32_t cellNo2, CellNos& dest)
{
	if (!_checkCellBound(cellNo1) || !_checkCellBound(cellNo2))
	{
		WARN_LOG("failed to cell bound [cellNo1: %d, cellNo2: %d]", cellNo1, cellNo2);
		return;
	}

	const auto& cellList1 = _cells[cellNo1].getNearCellList();
	const auto& cellList2 = _cells[cellNo2].getNearCellList();

	if (dest.size() < cellList1.size() + cellList2.size())
	{
		WARN_LOG("cellList capacity size error");
		return;
	}

	auto it = std::set_union(
		cellList1.begin(),
		cellList1.end(),
		cellList2.begin(),
		cellList2.end(),
		dest.begin());

	dest.erase(it, dest.end());
}
