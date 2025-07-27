


#include "Pch.h"
#include "AdminCmdTest.h"

#include "User/User.h"
#include "User/UserManager.h"

//#include "User/UserDBObject.h"
#include <Core/Task/TaskCaller.h>


AdminCmdTest g_AdminCmdTestInstance;


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		생성자
////////////////////////////////////////////////////////////////////////////////////////////////////
AdminCmdTest::AdminCmdTest()
	:
	IAdminCommand("test")
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		명령을 실행한다
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string AdminCmdTest::exec(const std::map<std::string, int64_t>& cmdParams)
{
	UserManager::Instance().foreach([](UserPtr user)
		{
			/*user->runTx( Caller, [ user ](Core::TaskTxPtr tx)
				{
					auto& dbObject = user->getDBObject();
					auto cacheObject = tx->acquireObject(&dbObject);
					cacheObject->lastConnectedTime = Core::Time::GetCurTime();
					cacheObject->update();

					return true;
				});*/
		});

	return "";
}
