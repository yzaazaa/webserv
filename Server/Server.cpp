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
# include "../Client/Client.hpp"
#include <exception>
#include <stdexcept>

/// *** Constructors *** ///
#pragma region Constructors

Server::Server()
{

}

#pragma endregion


/// *** Server Initialization Functions *** ///
#pragma region Server Initialization Functions

bool	Server::IsConnected(const ServerInstance& instance, const ListenInfo& listenInfo)
{
	for (SocketEntryIterator it = _socketEntrys.begin(); it != _socketEntrys.end(); ++it)
	{
		const SocketEntry& sEntry = it->second;

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


/// *** Server Loop Functions *** ///
#pragma region Server Loop Functions

void	Server::OnNewClientDetected(int kq, int serverFd)
{
	struct sockaddr_in	client_addr;
	socklen_t			client_len;
	int					client_fd;

	client_len = sizeof(client_addr);
	if ((client_fd = accept(serverFd, (struct sockaddr *)&client_addr, &client_len)) == -1)
		KqueueUtils::ThrowErrnoException("Error accepting client");
	else
	{
		// Register the client socket with kqueue for readability
		KqueueUtils::RegisterEvents(kq, client_fd);
		Client	new_client(client_fd, kq, serverFd);
		_clientMap[client_fd] = new_client;
		std::cout << "Accepted connection from " << inet_ntoa(client_addr.sin_addr) << std::endl;
	}
}

void	Server::OnClientDisconnected(int kq, int fd)
{
	std::cout << "Client disconnected." << std::endl;
	KqueueUtils::DeleteEvents(kq, fd);
	close(fd);
	_clientMap.erase(fd);
}

void	Server::OnFileDescriptorReadyForRead(int fd)
{
	std::cout << "File descriptor: " << fd << " ready for read." << std::endl;
	ClientMapIterator	it = _clientMap.find(fd);
	if (it != _clientMap.end())
	{
		it->second.readRequest(*this);
		it->second.parseRequest();
		it->second.clearRequestBuffer();
		it->second.changeClientState(PROCESSING);
		it->second.handleRequest();
		it->second.changeClientState(SENDING);
	}
	else
		throw std::runtime_error("Error retrieving client data.");
}

void	Server::OnFileDescriptorReadyForWrite(int kq, int fd)
{
	std::cout << "File descriptor: " << fd << " ready for write." << std::endl;
	ClientMapIterator	it = _clientMap.find(fd);
	if (it != _clientMap.end())
	{
		write(fd, it->second.getResponseBuffer().c_str(), it->second.getResponseLen());
		it->second.clearResponseBuffer();
		OnClientDisconnected(kq, fd);
	}
	else
		throw std::runtime_error("Error retrieving client data.");
}

#pragma endregion


/// *** Utility Functions *** ///
#pragma region Utility Functions

bool	Server::IsFileDescriptorServerSocket(int fd)
{
	for (SocketEntryIterator socketIt = _socketEntrys.begin(); socketIt != _socketEntrys.end(); ++socketIt)
	{
		SocketEntry& entry = socketIt->second;
		if (entry.SocketFd == fd)
			return (true);
	}
	return (false);
}

#pragma endregion


#pragma region Getters / Setters

SocketEntryDef&	Server::GetSocketEntrys()
{
	return (_socketEntrys);
}

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