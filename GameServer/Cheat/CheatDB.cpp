

#include "Pch.h"
#include "CheatDB.h"

#include <Core/Ini.h>

#include "Actor/Player.h"
#include "DB/CacheTx.h"

static CheatDB s_instanceCheatDB; ///< 인스턴스


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
CheatDB::CheatDB()
	:
	Cheat("db")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		핸들러
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatDB::onHandler(PlayerPtr player, const Core::StringList& params)
{
	if (params.empty())
	{
		WARN_LOG("invalid param.");
		return;
	}

	if (params[0] == "reset")
		return _reset(player, params);
	else if (params[0] == "info")
		return _info(player, params);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		리셋한다
////////////////////////////////////////////////////////////////////////////////////////////////////
void CheatDB::_reset(PlayerPtr player, const Core::StringList& params)
{
	INFO_LOG("db reset [playerId:%llu]", player->getId());

	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	cacheTx->pushDbTask([player](const DataBaseTarget& db)
		{
			if (!db.execute("show tables"))
			{
				WARN_LOG("show tables query is failed");
				return false;
			}

			ResultSet rs;
			db.populate(rs);

			std::vector<std::string> truncateQuerys;
			auto dbName = Core::Ini::Instance().getString("GAME_DB_NAME");

			std::vector<std::string> row;
			while (rs.fetch(row))
			{
				auto table = row[0];
				truncateQuerys.push_back(Core::StringUtil::ImplFormat("truncate %s.%s", dbName.c_str(), table.c_str()));
			}
			for (const auto& truncateQuery : truncateQuerys)
			{
				if (!db.execute(truncateQuery))
				{
					WARN_LOG("truncate query is failed");
					return false;
				}
			}

			return true;
		});

	cacheTx->ifSucceed(*player, [player]()
		{
#ifdef PLATFORM_WINDOWS
			TerminateProcess(GetCurrentProcess(), 0);
#else
			exit(0);
#endif
		});

	cacheTx->run();
}

void CheatDB::_info(PlayerPtr player, const Core::StringList& params)
{
	CacheTxPtr cacheTx = std::make_shared<CacheTx>(Caller);
	cacheTx->pushDbTask([player](const DataBaseTarget& db)
		{
			if (!db.execute("show variables like '%timeout'"))
			{
				WARN_LOG("show tables query is failed");
				return false;
			}

			ResultSet rs;
			db.populate(rs);

			std::vector<std::string> row;
			while (rs.fetch(row))
			{
				auto valueName = row[0];
				auto value = row[1];
				INFO_LOG("%s:%s", valueName.c_str(), value.c_str());
			}

			return true;
		});

	cacheTx->run();
}
