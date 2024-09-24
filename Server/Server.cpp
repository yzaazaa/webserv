/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 00:15:23 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/16 00:37:56 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Server.hpp"

/// *** Constructors *** ///
#pragma region Constructors

Server::Server()
{

}

#pragma endregion

/// *** Functions *** ///
#pragma region Functions

bool	Server::IsConnected(const ServerInstance& instance, const ListenInfo& listenInfo)
{
	for (SocketEntryIterator it = _socketEntrys.begin(); it != _socketEntrys.end(); ++it)
	{
		const SocketEntry& sEntry = it->second;
		//std::cout << "InMap: " << sEntry.SocketDomain << " " << sEntry.SocketType << " " << sEntry

		if (sEntry.Info == listenInfo)
		{
			AddServerInstance(instance, it->first);
			return (true);
		}
	}
	return (false);
}

void	Server::AddServerInstance(const ServerInstance& serverInstance, int socketFd)
{
	// Add server to the vector
	SocketEntryIterator it = _socketEntrys.find(socketFd);
    if (it == _socketEntrys.end())
		return;

	std::vector<ServerInstance>& serverInstances = it->second.ListeningServers;
	for (std::vector<ServerInstance>::iterator it = serverInstances.begin(); it != serverInstances.end(); ++it)
		if (*it == serverInstance)
			return;

	serverInstances.push_back(serverInstance);
}

SocketEntry&	Server::CreateSocket(const ServerInstance& instance, SocketEntry socketEntry)
{
	int socketFd = socket(socketEntry.SocketDomain, socketEntry.SocketType, socketEntry.SocketProtocol);
	if (socketFd < 0)
		throw std::runtime_error("Failed to create socket");

	try
	{
		socketEntry.SocketFd = socketFd;
		_socketEntrys[socketFd] = socketEntry;
		AddServerInstance(instance, socketFd);
		return (_socketEntrys[socketFd]);
	}
	catch (const std::exception& excep)
	{
		close(socketFd);
		throw;
	}
}


void	Server::ConnectSocket(int socketFd, struct sockaddr * address, int addressLen)
{
	/* Speed Up Starting Server */
	// Speed up restarting (allows using the same port/adress by multiple sockets)
	// TODO: is this required or i can bind multiple sockets with same port without it?
	int optval = 1;
	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof optval) < 0)
		throw std::runtime_error("Failed to attach socket.");

	if (bind(socketFd, address, addressLen) < 0)
		throw std::runtime_error("Failed to bind socket.");
	
	if (listen(socketFd, SOMAXCONN) < 0)
		throw std::runtime_error("Socket listen failed.");
}

#pragma endregion

/// *** Exception *** ///
#pragma region Exception


#pragma endregion

/// *** Operators *** ///
#pragma region Operators

#pragma endregion

/// *** Destructor *** ///
#pragma region Destructor

Server::~Server()
{
	for (SocketEntryIterator it = _socketEntrys.begin(); it != _socketEntrys.end(); ++it)
	{
		SocketEntry& socketEntry = it->second;
		if (socketEntry.SocketFd >= 0)
			close(socketEntry.SocketFd);
	}
}

#pragma endregion