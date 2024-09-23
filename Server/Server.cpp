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
#include "../Client/Client.hpp"

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

void	Server::registerServerSockets(int kq)
{
	for (SocketEntryIterator it = _socketEntrys.begin(); it != _socketEntrys.end(); it++)
	{
		it->second.registerServerSocket(kq);
	}
}

ClientMapIterator	Server::findClient(int fd)
{
	return _clientMap.find(fd);
}

void	Server::disconnectClient(ClientMapIterator it)
{
	std::cout << "Client disconnected." << std::endl;
	it->second.disconnect();
	_clientMap.erase(it);
}

void	Server::disconnectClient(Client &client)
{
	std::cout << "Client disconnected." << std::endl;
	client.disconnect();
	_clientMap.erase(_clientMap.find(client.getClientSocket()));
}

SocketEntryIterator	Server::findServer(int fd)
{
	return _socketEntrys.find(fd);
}

SocketEntryIterator	Server::getServerMapEnd()
{
	return _socketEntrys.end();
}

void	Server::acceptNewClient(int fd, int kq)
{
	int	client_socket = accept(fd, (struct sockaddr *)NULL, NULL);
	Client	new_client(client_socket, kq);
	_clientMap[client_socket] = new_client;
	std::cout << "New client connected." << std::endl;
}

void	Server::sendResponse(Client &client)
{
	std::cout << "Sending response: " << std::endl << client.getResponseBuffer() << std::endl;
	write(client.getClientSocket(), client.getResponseBuffer().c_str(), client.getResponseLen());
	client.clearResponseBuffer();
	disconnectClient(client);
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