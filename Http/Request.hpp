#pragma once
# include <iostream>
# include <map>
#include <string>

struct URI
{
	URI() { Reset(); }

	std::string path;
	std::string	extension;
	std::string query;

	void	Reset()
	{
		path.clear();
		query.clear();
	}
};

struct Request
{
	Request() : IsHeaderParsingDone(false) { Reset(); }

	std::string	Buffer;
	std::string method;
	URI			uri;
	std::string http_version;
	std::map<std::string ,std::string> headers;
	std::string body;
	bool			IsHeaderParsingDone;

	void	Reset()
	{
		Buffer.clear();
		method.clear();
		uri.Reset();
		http_version.clear();
		headers.clear();
		body.clear();
		IsHeaderParsingDone = false;
	}
};
