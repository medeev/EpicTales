////////////////////////////////////////////////////////////////////////////////////////////////////
/// @ brief		관리자클래스
///
///	@ date		2024-3-6
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <Asio/Service/Http.h>

class IAdminCommand;
class AdminManager : public Core::Singleton<AdminManager>
{

public:
	/// 소멸자
	~AdminManager();

	/// 명령을 추가한다
	void addCommand(const std::string& cmd, IAdminCommand* object);

	/// 명령을 실행한다
	std::string exec(std::string cmd, const std::map<std::string, int64_t>& cmdParams);

protected:
	std::map<std::string, IAdminCommand*> _commands;		///< 명령어 목록

public:
};


inline bool url_decode(const std::string& in, std::string& out)
{
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] == '%')
		{
			if (i + 3 <= in.size())
			{
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value)
				{
					out += static_cast<char>(value);
					i += 2;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else if (in[i] == '+')
		{
			out += ' ';
		}
		else
		{
			out += in[i];
		}
	}
	return true;
}

inline void _parseParams(const std::string& paramPart, std::map<std::string, int64_t>& paramList)
{
	for (const auto& eachParam : Core::StringUtil::Split(paramPart, "&"))
	{
		auto paramTokens = Core::StringUtil::Split(eachParam, "=");
		if (paramTokens.empty())
			continue;

		auto paramName = paramTokens[0];
		int64_t paramValue = 0;
		if (paramTokens.size() > 1)
			paramValue = std::strtoll(paramTokens[1].c_str(), nullptr, 10);

		paramList.insert(std::make_pair(paramName, paramValue));
	}
}


inline http::status _parseRequest(
	const http::request<http::string_body>& req,
	std::string& cmd,
	std::map<std::string, int64_t>& paramList)
{
	// Decode url to path.
	std::string requestPath;
	if (!url_decode(req.target(), requestPath))
		return http::status::bad_request;

	INFO_LOG(requestPath.c_str());

	if (
		requestPath.empty() ||
		requestPath[0] != '/' ||
		requestPath.find("..") != std::string::npos)
		return http::status::bad_request;

	auto findSlash = requestPath.find_last_of('/');
	if (findSlash != std::string::npos)
		cmd = requestPath.substr(findSlash + 1);

	auto questionIndex = requestPath.find('?');
	if (questionIndex != std::string::npos)
		_parseParams(requestPath.substr(questionIndex + 1), paramList);

	if (req.method() == http::verb::post) {
		// POST 요청 본문 데이터 확인
		const std::string& postData = req.body();

		// POST 데이터 출력
		_parseParams(postData, paramList);
	}

	return http::status::ok;
}


//{ "css", "text/css" },
//{ "gif", "image/gif" },
//{ "htm", "text/html" },
//{ "html", "text/html" },
//{ "jpg", "image/jpeg" },
//{ "png", "image/png" },
//{ "xml", "text/xml" }

inline http::response<http::string_body> handleRequest(const http::request<http::string_body>& req)
{
	/*
	// HTTP 메서드 출력
	std::cout << "Method: " << req.method_string() << std::endl;

	// 요청 대상(URL) 출력
	std::cout << "Target: " << req.target() << std::endl;

	// HTTP 버전 출력
	std::cout << "Version: " << req.version() << std::endl;

	// 헤더 출력
	std::cout << "Headers: " << std::endl;
	for (const auto& field : req.base()) {
		std::cout << field.name_string() << ": " << field.value() << std::endl;
	}

	// 바디 출력
	std::cout << "Body: " << req.body() << std::endl;
	*/


	http::response<http::string_body> res{ http::status::ok, req.version() };

	std::string cmd;
	std::map<std::string, int64_t> cmdParams;
	auto result = _parseRequest(req, cmd, cmdParams);
	res.result(result);
	if (result == http::status::ok)
		res.body() = AdminManager::Instance().exec(cmd, cmdParams);
	else
		res.body() = "<html><body><h1>Hello from Boost.Beast HTTPS Server!</h1></body></html>";
	
	res.set(http::field::server, "Boost.Beast");
	res.set(http::field::content_type, "text/plain");
	
	res.prepare_payload();
	return res;
}

