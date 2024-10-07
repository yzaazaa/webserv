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
# include <sys/types.h>
# include <sys/event.h>
# include <sys/time.h>
# include <cerrno>
# include <iostream>
# include "../KqueueUtils/KqueueUtils.hpp"
# include <unistd.h>

# define TIMEOUT 10000

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

	int	SocketFd; 
	int	SocketDomain;
	int	SocketType;
	int	SocketProtocol;
	ListenInfo Info;
	/// Servers that listen on this socket
	std::vector<ServerInstance> ListeningServers;
};

class	Client;

typedef	std::map<int, SocketEntry> SocketEntryDef;
typedef	std::map<int, SocketEntry>::iterator SocketEntryIterator;
typedef	std::map<int, Client>::iterator ClientMapIterator;
typedef	std::map<int, int>::iterator FdMapIterataor;


class	Server
{
	private:
		std::map<int, SocketEntry>	_socketEntrys;
		std::map<int, Client>		_clientMap;
		std::map<int, int>			_fdMap;
		char**						_env;

		/// @brief Adds a ServerInstance to the SocketEntry if not already present.
		void	AddServerInstance(const ServerInstance& serverInstance, int socketFd);


	public:
		// Constructors
		Server(char **env);

		// Operators
		Server& operator=(const Server& other);

#pragma region Functions

		/// *** Server Initialization Functions *** ///
		SocketEntry&	CreateSocket(const ServerInstance& instance, SocketEntry socketEntry);
		bool			IsConnected(const ServerInstance& instance, const ListenInfo& listenInfo);
		void			ConnectSocket(int socketFd, struct sockaddr * address, int addressLen);


		/// *** Server Loop Functions *** ///
		/// @brief kicks afk clients and monitors cgi timeout
		void	monitorActivty(int kq);
		/// @brief Handles when a new client wants to connect
		void	OnNewClientDetected(int kq, int serverFd);
		/// @brief Handles when a client disconnects
		void	OnClientDisconnected(int kq, int fd);
		/// @brief Handles when a file descriptor is ready to be read from
		void	OnFileDescriptorReadyForRead(int kq, int fd);
		/// @brief Handles when a file descriptor is ready to be written into
		void	OnFileDescriptorReadyForWrite(int kq, int fd);

		void	addFd(int fd, int socket);
		void	eraseFd(int fd);
	

		/// *** Utility Functions *** ///
		/// @brief Returns true if the file descriptor belongs to a server listening socket
		bool	IsFileDescriptorServerSocket(int fd);
		char	**getEnv();

#pragma endregion

		// Getters
		SocketEntryDef& GetSocketEntrys();

		// Destructor
		~Server();
};

long	ft_time(void);