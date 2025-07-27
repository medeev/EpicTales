

#include "Pch.h"
#include "GameServer.h"

#include <Core/Port/PortLayer.h>
#include <Core/Util/AppInstance.h>
#include <Core/Util/IdFactory.h>
#include <Core/Ini.h>

#include <Data/Info/InfoLoader.h>

#ifdef PLATFORM_WINDOWS
#pragma warning( disable: 4091 )
#pragma warning( disable: 4267 )
#endif

int main(int32_t argc, char** argv)
{
	Core::AppInstance::DrawBox(80, 10, "EpicTales Game Server is created by medeev\n");

#ifdef PLATFORM_WINDOWS
#ifdef _DEBUG
	// Get the handle to the console window
	HWND console = GetConsoleWindow();

	// Set the width and height of the console window
	RECT rect;
	GetWindowRect(console, &rect);
	MoveWindow(console, rect.left, rect.top, 1280, 520, TRUE);
#else
	Core::AppInstance::StartMiniDump();
#endif
#endif
	Core::AppInstance::SetBundlePath(argv[0]);

	std::string fileName = Core::StringUtil::ExtractDriveDir(
		Core::AppInstance::GetBundlePath()) +
		Core::StringUtil::ExtractName(argv[0]) + ".ini";
	
	if (!Core::Ini::Instance().load(fileName))
	{
		std::cout << "failed to load " << fileName << std::endl;
		return 1;
	}

	Core::Log::Initialize(
		Core::StringUtil::ExtractDriveDir(
			Core::AppInstance::GetBundlePath()) + Core::Ini::Instance().getString("LOG_PATH"),
		Core::Ini::Instance().getString("LOG_NAME"));

	Core::Log::SetAsyncWriteEnabled(true);

	Core::IdFactory::Initialize(Core::BaseServer::GetServerId());

	InfoLoader::Setup(Core::AppInstance::GetDataPath() + "/Xlsx/");

	GameServer::Instance().start(Config::GetIOCPThreadCount());

	Core::Log::Finalize();

	return 0;
}
