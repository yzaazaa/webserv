#pragma once
# include <iostream>
# include <map>

struct	Response
{
	Response() : IsLastResponse(false), CloseConnection(false) { Reset(); }

	std::string	Buffer;
	bool		IsLastResponse;
	bool		CloseConnection;

	void	Reset()
	{
		Buffer.clear();
		IsLastResponse = false;
		CloseConnection = false;
	}
};
