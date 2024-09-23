/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:06:39 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/23 18:51:19 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../Parser/Parser.hpp"
# include "../Server/Server.hpp"
# include "../Server/ServerInitializer.hpp"
#include <cstddef>
# include <cstdlib>
#include <sys/event.h>
# include "../Debug/Debug.hpp"
#include "../Client/Client.hpp"

# define MAX_EVENTS SOMAXCONN

int	main(int argc, char **argv)
{
	try
	{
		Parser parser(argc, argv);
		Server server;
		//PrintParsedServer(parser);
		ServerInitializer serverInitializer(server, parser);
		int	kq = kqueue();
		if (kq == -1)
		{
			perror("webserv: ");
			return (1);
		}
		server.registerServerSockets(kq);
		struct kevent	evList[MAX_EVENTS];
		while (667)
		{
			int nev = kevent(kq, NULL, 0, evList, sizeof(evList), NULL);
			if (nev == -1)
			{
				perror("webserv: ");
				close(kq);
				return (1);
			}
			for (int i = 0; i < nev; i++)
			{
				int	socket_fd = evList[i].ident;
				ClientMapIterator it= server.findClient(socket_fd);
				if (evList[i].flags & EV_EOF)
					server.disconnectClient(it);
				else if (evList[i].filter == EVFILT_READ)
				{
					if (server.findServer(socket_fd) != server.getServerMapEnd())
						server.acceptNewClient(socket_fd, kq);
					else if (it->second.getClientState() == READING)
					{
						it->second.readRequest(server);
						it->second.parseRequest();
						it->second.clearRequestBuffer();
						it->second.changeClientState(PROCESSING);
						it->second.handleRequest();
						it->second.changeClientState(SENDING);
					}
				}
				else if (evList[i].filter == EVFILT_WRITE && it->second.getClientState() == SENDING)
					server.sendResponse(it->second);
			}
		}
		close(kq);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}