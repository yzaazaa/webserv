#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include "Client.hpp"

# define MAX_EVENTS SOMAXCONN

void	register_socket(int socket, int kq)
{
	struct kevent evSet;
	EV_SET(&evSet, socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	kevent(kq, &evSet, 1, NULL, 0, NULL);
}

int	main()
{
	std::map<int, Client>	client_map;
	int	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	std::cout << "Server socket created: " << server_socket << std::endl;
	int	opt = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
	struct sockaddr_in	server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8080);
	inet_pton(AF_INET, "0.0.0.0", &server_address.sin_addr);
	std::cout << "Setting up server config." << std::endl;
	bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
	std::cout << "Server binded." << std::endl;
	listen(server_socket, MAX_EVENTS);
	int	kq = kqueue();
	if (kq == -1)
	{
		perror("webserv: ");
		return (1);
	}
	struct kevent	evList[MAX_EVENTS];
	register_socket(server_socket, kq);
	while (true)
	{
		int nev = kevent(kq, NULL, 0, evList, sizeof(evList), NULL);
		if (nev == -1)
		{
			perror("webserv: ");
			return (1);
		}
		for (int i = 0; i < nev; i++)
		{
			int socket_fd = evList[i].ident;
			std::map<int, Client>::iterator it = client_map.find(socket_fd);
			if(evList[i].flags == EV_EOF)
			{
				std::cout << "client disconnected." << std::endl;
				it->second.disconnect();
				client_map.erase(it);
			}
			else if (evList[i].filter == EVFILT_READ)
			{
				if (socket_fd == server_socket)
				{
					int	client_socket = accept(server_socket, (struct sockaddr *)NULL, NULL); 
					Client	new_client(client_socket, kq);
					client_map[client_socket] = new_client;
					std::cout << "New client connected." << std::endl;
				}
				else if (it->second.getClientState() == READING)
				{
					char	message[1024];
					size_t	bytes_read = read(socket_fd, message, 1024);
					if (bytes_read <= 0)
					{
						std::cout << "client disconnected." << std::endl;
						it->second.disconnect();
						client_map.erase(it);
						continue ;
					}
					it->second.appendRequestBuffer(message);
					std::cout << "Request from client: " << std::endl << it->second.getRequestBuffer() << std::endl;
					it->second.clearRequestBuffer();
					it->second.changeClientState(PROCESSING);
					it->second.appendResponseBuffer("Message received\r\n");
					it->second.changeClientState(SENDING);
				}
			}
			else if (evList[i].filter == EVFILT_WRITE && it->second.getClientState() == SENDING)
			{
				write(socket_fd, it->second.getResponseBuffer().c_str(), it->second.getResponseLen());
				it->second.clearResponseBuffer();
				it->second.changeClientState(READING);
			}
		}
	}
	close(kq);
}