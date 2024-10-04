# include "HeaderValidator.hpp"
#include <cctype>
# include <cstddef>
# include <iostream>
# include <sstream>
# include <string>

/// *** Constructors *** ///
#pragma region Constructors

HeaderValidator::HeaderValidator()
{

}

HeaderValidator::HeaderValidator(const HeaderValidator& other)
{
	*this = other;
}

#pragma endregion

/// *** Part 1, Read and parse the header *** ///
#pragma region Part 1, Read and parse the header

void HeaderValidator::DEBUG_PRINT_REQUEST(Request request)
{
	std::cout << "***************************************************" << std::endl;
	std::cout << "Method: " << request.method << std::endl;
	std::cout << "URI: " << request.uri.path << std::endl;
	std::cout << "HTTP Version: " << request.http_version << std::endl;
	std::cout << "Headers:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = request.headers.begin(); it != request.headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << "Body: " << request.body << std::endl;
	std::cout << "***************************************************" << std::endl;
}

bool	HeaderValidator::ParseHeaderLine(const std::string& line, Request &request, Response& response)
{
	std::istringstream	line_stream(line);
	std::string			uri;
	std::size_t			pos;

	if (!(line_stream >> request.method >> uri >> request.http_version))
		return (ResponseUtils::BadRequest400_NoBody(response), false);

	if (uri.size() > URI_MAX_SIZE)
		return (ResponseUtils::URITooLong414_NoBody(response), false);

	for (size_t i = 0; i < request.method.size(); i++)
		request.method[i] = std::tolower(request.method[i]);
	if (request.method != "get" && request.method != "post" && request.method != "delete")
		return (ResponseUtils::NotImplemented501_NoBody(response), false);

	pos = uri.find("?");
	request.uri.path = uri;
	request.uri.query = "";
	if (pos != std::string::npos)
	{
		request.uri.path = uri.substr(0, pos);
		request.uri.query = uri.substr(pos + 1);
	}
	// if (request.uri.path[0] != '/') 
	// 	request.uri.path.insert(0, "/");

	for (size_t i = 0; i < request.uri.path.size(); i++)
	{
		if (!std::isalnum(request.uri.path[i]) && !(std::strchr("-._~:/?#[]@!$&'()*+,;=", request.uri.path[i])))
			return (ResponseUtils::BadRequest400_NoBody(response), false);
	}
	return (true);
}

bool	HeaderValidator::ParseHeaders(const std::string& line, Request &request)
{
	std::string::size_type colon_pos;
	std::string header;
	std::string value;

	colon_pos = line.find(":");
	if (colon_pos == std::string::npos)
		return (false);
	header = line.substr(0, colon_pos);
	if (line[colon_pos + 1] == ' ')
		value = line.substr(colon_pos + 2);
	else
		value = line.substr(colon_pos + 1);
	if (!value.empty() && value.back() == '\r')
        value.erase(value.length() - 1);
	for (size_t i = 0; i < header.size(); i++)
		header[i] = std::tolower(header[i]);
	if (request.headers.find(header) != request.headers.end())
		return (false);
	request.headers[header] = value;
	return (true);
}

bool	HeaderValidator::ParseHeader(Request &request, Response& response)
{
	std::istringstream request_stream(request.Buffer);
	std::string line;

	if (!std::getline(request_stream, line))
		return (ResponseUtils::BadRequest400_NoBody(response), false);
	if (!ParseHeaderLine(line, request, response))
		return (false); //Response already created, DON NOT Override it
	while (std::getline(request_stream, line) && line != "\r")
	{
		if (!ParseHeaders(line, request))
			return (ResponseUtils::BadRequest400_NoBody(response), false);
	}
	request.IsHeaderParsingDone = true;
	DEBUG_PRINT_REQUEST(request);
	return (true);
}

bool	HeaderValidator::ReadHeader(Request &request, Response& response, int fd)
{
	char	message[READING_BUFFER_SIZE];
	size_t	bytes_read = recv(fd, message, READING_BUFFER_SIZE, 0);

	if (bytes_read < 0)
		return (ResponseUtils::InternalServerError500_NoBody(response), false);
	message[bytes_read] = 0;
	request.Buffer += message;
	if (request.Buffer.size() > HEADER_MAX_SIZE)
		return (ResponseUtils::BadRequest400_NoBody(response), false);
	return (true);
}

bool	HeaderValidator::ReadAndParseHeader(Client& client, Server& server, int fd)
{
	if (!client.Request.IsHeaderParsingDone)
	{
		if (!HeaderValidator::ReadHeader(client.Request, client.Response, fd))
			return (false);

		// Wait untill the client has sent the entire header
		if (client.Request.Buffer.find(DoubleEndl_Request) != std::string::npos)
		{
			if (!HeaderValidator::ParseHeader(client.Request, client.Response))
				return (false);

			if (!ResolveRequestContext(client, server))
				return (false);
		}
	}

	return (true);
}

void	HeaderValidator::RemoveHeaderFromBuffer(Request& request)
{
	std::size_t found = request.Buffer.find("\r\n\r\n");
	if (found != std::string::npos)
		request.Buffer.erase(0, found + 4);
	std::cout << "this is what left in the request buffer: " << request.Buffer << std::endl;
}

#pragma endregion

/// *** Part 2, Resolve request context, and get Server/Location Instances *** ///
#pragma region Part 2, Resolve request context, and get Server/Location Instances

ServerInstance*	HeaderValidator::GetServerInstance(Request& request, Server& server, int serverFd)
{
	std::string	host;
	std::vector<ServerInstance>& listeningServers = server.GetSocketEntrys()[serverFd].ListeningServers;
	if (listeningServers.size() <= 0)
		return (NULL);

	if (request.headers.find("host") != request.headers.end())
	{
		host = request.headers["host"];
		for (std::vector<ServerInstance>::iterator srvIt = listeningServers.begin(); srvIt != listeningServers.end(); ++srvIt)
		{
			ServerInstance& svInst = *srvIt;
			if (svInst.ServerNames.find(host) != svInst.ServerNames.end())
				return (&svInst);
		}
	}

	// Return the default server for this Adress:port
	return (&listeningServers[0]);
}

LocationInstance*	HeaderValidator::GetLocationInstance(Request& request, ServerInstance& serverInstance)
{
	size_t	highestMatchLength = 0;
	LocationInstance*	locationInstance = NULL;

	for (LocationsIter	locIt = serverInstance.Locations.begin(); locIt != serverInstance.Locations.end(); ++locIt)
	{
		LocationInstance& locInstance = locIt->second;
		if (request.uri.path.size() < locInstance.PathArg.size())
			continue;

		if (locInstance.IsExactMatch)
		{
			if (locInstance.PathArg == request.uri.path)
				return (&locIt->second);
			continue;
		}
	
		size_t currentLen = 0;
		while (currentLen < locInstance.PathArg.size() && locInstance.PathArg[currentLen] == request.uri.path[currentLen])
			currentLen++;
		
		if (currentLen > highestMatchLength)
		{
			highestMatchLength = currentLen;
			locationInstance = &locInstance;
		}
	}

	return (locationInstance);
}

bool	HeaderValidator::ResolveRequestContext(Client& client, Server& server)
{
	ServerInstance*		serverInstance = GetServerInstance(client.Request, server, client.ServerFd);
	if (!serverInstance)
		return (ResponseUtils::NotFound404_NoBody(client.Response), false);

	client.Location = GetLocationInstance(client.Request, *serverInstance);
	if (!client.Location)
		return (ResponseUtils::NotFound404_NoBody(client.Response), false);

	if (client.Location->Redirection.IsDefined)
	{
		//if (client.Location->Redirection.Code == 301) //TODO; i dont know how this should work ...
			return (ResponseUtils::MovedPermanently301_NoBody(client.Response, client.Location->Redirection.Value), false);
		
		// Will mostlikey either return the value as a body if its not a link?
	}

	if (client.Location->AllowedMethods.find(client.Request.method) == client.Location->AllowedMethods.end())
	{
		if (client.Location->AllowedMethods.size() > 0)
			return (ResponseUtils::MethodNotAllowed405_NoBody(client.Response), false);
	}

	if (!client.Location->RootDirectory.IsDefined)
		return (ResponseUtils::NotFound404_NoBody(client.Response), false);

	
	return  (true);
}

#pragma endregion

/// *** Destructor *** ///
#pragma region Destructor

HeaderValidator::~HeaderValidator()
{

}

#pragma endregion
