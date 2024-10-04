#pragma once
# include "../Request.hpp"
# include <iostream>
# include "../ResponseUtils/ResponseUtils.hpp"
# include <unistd.h>
# include "../Client.hpp"

#define URI_MAX_SIZE 2048
#define HEADER_MAX_SIZE 9216

class HeaderValidator
{
	private:
		HeaderValidator();
		HeaderValidator(const HeaderValidator& other);

		// Functions
		static bool	ParseHeaderLine(const std::string& line, Request& request, Response& response);
		static bool	ParseHeaders(const std::string& line, Request& request);
		static void	DEBUG_PRINT_REQUEST(Request request); // test function
	
		static bool ParseHeader(Request& request, Response& response);
		static bool ReadHeader(Request& request, Response& response, int fd);

		/// @brief Resolves the Location/Server Instance handler for a given request
		static bool	ResolveRequestContext(Client& client, Server& server);
		static ServerInstance*		GetServerInstance(Request& request, Server& server, int serverFd);
		static LocationInstance*	GetLocationInstance(Request& request, ServerInstance& serverInstance);

	public:
		// Functions
		static bool	ReadAndParseHeader(Client& client, Server& server, int fd);
		static void	RemoveHeaderFromBuffer(Request& request);


		// Destructor
		~HeaderValidator();
};
