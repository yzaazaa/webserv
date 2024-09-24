/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInitializer.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/17 03:11:51 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/23 14:56:57 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ServerInitializer.hpp"

/// *** Constructors *** ///
#pragma region Constructors

ServerInitializer::ServerInitializer() : _isAnySocketOpen(false)
{	}

ServerInitializer::ServerInitializer(const ServerInitializer& other) : _isAnySocketOpen(false)
{
	*this = other;
}

ServerInitializer::ServerInitializer(Server& server, Parser& parser) : _isAnySocketOpen(false)
{
	try
	{
		InitializeServer(server, parser);
		if (!_isAnySocketOpen)
			throw std::runtime_error("Server failed to start: No available or open listening socket.");
	}
	catch (const std::exception& e)
	{
		throw;
	}
	
}

#pragma endregion

/// *** Functions *** ///
#pragma region Functions

void	ServerInitializer::ConnectServers(Server& server, ServerInstance& instance, const ListenInfo& listenInfo)
{
	addrinfo	hints;
	addrinfo*	cAddr;
	addrinfo*	res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int	status = getaddrinfo(listenInfo.Address.c_str(), listenInfo.Port.c_str(), &hints, &res);
	if (status != 0)
	{
		std::cerr << "\033[94mTrying to open:\033[0m " << "[" << listenInfo.ToString() << "]: ";
		std::cerr << "\033[31mError\033[0m" << std::endl;
		std::cerr << "\t\tgetaddrinfo error: " << gai_strerror(status) << std::endl;
		return;
	}
	
	for (cAddr = res; cAddr != NULL; cAddr = cAddr->ai_next)
	{
		std::cout << "\033[94m\nTrying to open:\033[0m " << "[" << listenInfo.ToString() << "]:";
		if (cAddr->ai_family == AF_INET)
			std::cout << "IPv4";
		else
			std::cout << "IPv6";
		try
		{
			SocketEntry socketEntry(-1, cAddr->ai_family, cAddr->ai_socktype, cAddr->ai_protocol, listenInfo);
			SocketEntry& newSEntry = server.CreateSocket(instance, socketEntry);
			server.ConnectSocket(newSEntry.SocketFd, cAddr->ai_addr, cAddr->ai_addrlen);
		
			_isAnySocketOpen = true;
			std::cout<< " \033[32mOpen\033[0m";
		}
		catch (const std::exception& e)
		{
			std::cerr << " \033[31mError\033[0m" << std::endl;
			std::cerr << "\t\tException: " << e.what() << std::endl;
			std::cerr << "\t\tstrerror: " << strerror(errno) << std::endl;
		}
	}
	freeaddrinfo(res);
}

void	ServerInitializer::InitializeServer(Server& server, Parser& parser)
{
	std::map<int, ServerInstance>& servers = parser.GetParsedServers();
	for (ServerInstanceIterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt)
    {
		ServerInstance& instance = serverIt->second;
		for (ListenInfoIterator it = instance.ServerSockets.begin(); it != instance.ServerSockets.end(); ++it)
		{
			if (server.IsConnected(instance, *it))
				continue;
			ConnectServers(server, instance, *it);
		}
	}
}

#pragma endregion

/// *** Operators *** ///
#pragma region Operators

ServerInitializer& ServerInitializer::operator=(const ServerInitializer& other)
{
	if (this != &other)
	{
		
	}
	return *this;
}

#pragma endregion

/// *** Destructor *** ///
#pragma region Destructor

ServerInitializer::~ServerInitializer()
{

}

#pragma endregion