/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:06:39 by yzirri            #+#    #+#             */
/*   Updated: 2024/10/02 20:23:02 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../Parser/Parser.hpp"
# include "../Server/Server.hpp"
# include "../Server/ServerInitializer.hpp"
# include "../Debug/Debug.hpp"
# include <cerrno>
# include <cstddef>
# include <cstdlib>
#include <exception>
# include <iostream>
#include <iterator>
# include <unistd.h>
# include <sstream>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/event.h>
# include <sys/time.h>
# include "../KqueueUtils/KqueueUtils.hpp"

int	main(int argc, char **argv)
{
	Server	server;
	int		eventCount;
	int		kq;
	int		fd;

	try
	{
		Parser parser(argc, argv);
		//PrintParsedServer(parser);
		ServerInitializer serverInitializer(server, parser);
		kq = KqueueUtils::PrepareKqueue(server);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (EXIT_FAILURE);
	}

	
	struct kevent	evList[SOMAXCONN];
	while (667)
	{
		if ((eventCount = KqueueUtils::WaitForEvent(kq, evList, SOMAXCONN)) == -1)
			continue;
		
		for (int i = 0; i < eventCount; i++)
		{

			try
			{
				fd = evList[i].ident;
				// Action happend on a server listening socket
				if (server.IsFileDescriptorServerSocket(fd))
					server.OnNewClientDetected(kq, fd);
				
				// Client Disconnected
				else if (evList[i].flags & EV_EOF)
					server.OnClientDisconnected(kq, fd);
				
				// Client Can be Read
				else if (evList[i].filter == EVFILT_READ)
					server.OnFileDescriptorReadyForRead(kq, fd);
				
				// Client can be written to
				else if (evList[i].filter == EVFILT_WRITE)
					server.OnFileDescriptorReadyForWrite(kq, fd);
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << std::endl; //Don't let the server crash, just log the error and keep going
			}
		}
	}
	close(kq);
	return (EXIT_SUCCESS);
}