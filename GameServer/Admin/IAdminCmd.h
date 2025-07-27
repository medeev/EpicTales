////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		관리명령 인터페이스
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

class IAdminCommand
{
public:
	/// 생성자
	IAdminCommand(const std::string& cmd);

	/// 소멸자
	virtual ~IAdminCommand() {}

	/// 명령을 실행한다
	virtual std::string exec(const std::map<std::string, int64_t>& cmdParams) = 0;

	/// 플레이어식별자를 반환한다.
	uint64_t getPlayerId(const std::map<std::string, int64_t>& cmdParams) const;

	/// 유저식별자를 반환한다.
	uint64_t getUserId(const std::map<std::string, int64_t>& cmdParams) const;
};