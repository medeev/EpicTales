// pch.cpp: 미리 컴파일된 헤더에 해당하는 소스 파일

#include "Pch.h"

#if defined PLATFORM_WINDOWS

#pragma comment( lib, "Core.lib" )
#pragma comment( lib, "Asio.lib" )
#pragma comment( lib, "DB.lib" )
#pragma comment( lib, "Data.lib" )
#pragma comment( lib, "Protocol.lib" )
#pragma comment( lib, "libmysql.lib" )

#endif

// 미리 컴파일된 헤더를 사용하는 경우 컴파일이 성공하려면 이 소스 파일이 필요합니다.
