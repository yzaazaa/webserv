/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 00:15:10 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/16 01:09:32 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include <unistd.h>
# include <map>
# include "../ServerInstance/ServerInstance.hpp"
#include <sys/event.h>


struct	SocketEntry
{
	SocketEntry() :
		SocketFd(-1), SocketDomain(-1),
		SocketType(-1), SocketProtocol(-1)
	{ }
	SocketEntry(int socketFd, int socketDomain, int socketType, int socketProtocol, const ListenInfo& info) :
		SocketFd(socketFd), SocketDomain(socketDomain),
		SocketType(socketType), SocketProtocol(socketProtocol),
		Info(info)
	{ }
	void	registerServerSocket(int kq)
	{
		struct kevent evSet;
		EV_SET(&evSet, SocketFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		kevent(kq, &evSet, 1, NULL, 0, NULL);
	}

	int	SocketFd;
	int	SocketDomain;
	int	SocketType;
	int	SocketProtocol;
	ListenInfo Info;
	/// Servers that listen on this socket
	std::vector<ServerInstance> ListeningServers;
};

typedef	std::map<int, SocketEntry>::iterator SocketEntryIterator;

class	Client;

typedef	std::map<int, Client>::iterator ClientMapIterator;


class	Server
{
	private:
		std::map<int, SocketEntry> _socketEntrys;
		std::map<int, Client> _clientMap;

		/// @brief Adds a ServerInstance to the SocketEntry if not already present.
		void	AddServerInstance(const ServerInstance& serverInstance, int socketFd);


	public:
		// Constructors
		Server();

		// Operators
		Server& operator=(const Server& other);

		// Functions
		SocketEntry&		CreateSocket(const ServerInstance& instance, SocketEntry socketEntry);
		bool				IsConnected(const ServerInstance& instance, const ListenInfo& listenInfo);
		void				ConnectSocket(int socketFd, struct sockaddr * address, int addressLen);
		void				registerServerSockets(int kq);
		ClientMapIterator	findClient(int fd);
		SocketEntryIterator	findServer(int fd);
		void				disconnectClient(ClientMapIterator it);
		void				disconnectClient(Client &client);
		SocketEntryIterator	getServerMapEnd();
		void				acceptNewClient(int fd, int kq);
		void				sendResponse(Client &client);				


		// Destructor
		~Server();
};
