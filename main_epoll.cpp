#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <map>
#include "Client.hpp"

# define MAX_EVENTS SOMAXCONN

void	register_read(int socket, int epoll_fd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &event);
}

void	register_write(int socket, int epoll_fd)
{
    struct epoll_event event;
    event.events = EPOLLOUT;
    event.data.fd = socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket, &event);
}

void delete_write(int socket, int epoll_fd) {
    struct epoll_event event;
    event.events = EPOLLIN;  // Keep read events
    event.data.fd = socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket, &event);
}

void delete_read(int socket, int epoll_fd) {
    struct epoll_event event;
    event.events = EPOLLOUT;  // Keep write events
    event.data.fd = socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket, &event);
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
	int	epoll_fd = epoll_create1(0);
	struct epoll_event	events[MAX_EVENTS];
	register_read(server_socket, epoll_fd);
	while (true)
	{
		int nev = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		for (int i = 0; i < nev; i++)
		{
			int socket_fd = events[i].data.fd;
			std::map<int, Client>::iterator it = client_map.find(socket_fd);
			if (it == client_map.end() && socket_fd != server_socket)
			{
				std::cerr << "Client not found." << std::endl;
				continue ;
			}
			if(events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
			{
				std::cout << "client disconnected." << std::endl;
				delete_write(socket_fd, epoll_fd);
				delete_read(socket_fd, epoll_fd);
				close(socket_fd); 
				client_map.erase(it);
			}
			else if (events[i].events & EPOLLIN)
			{
				if (socket_fd == server_socket)
				{
					int	client_socket = accept(server_socket, (struct sockaddr *)NULL, NULL); 
					Client	new_client(client_socket);
					client_map[client_socket] = new_client;
					std::cout << "New client connected." << std::endl;
					register_read(client_socket, epoll_fd);
				}
				else if (it->second.getClientState() == READING)
				{
					char	message[1024];
					size_t	bytes_read = read(socket_fd, message, 1024);
					if (bytes_read <= 0)
					{
						std::cout << "client disconnected." << std::endl;
						delete_read(socket_fd, epoll_fd);
						delete_write(socket_fd, epoll_fd);
						close(socket_fd);
						client_map.erase(it);
                        continue ;
					}
					it->second.appendRequestBuffer(message);
					std::cout << "Request from client: " << std::endl << it->second.getRequestBuffer() << std::endl;
					it->second.clearRequestBuffer();
					delete_read(socket_fd, epoll_fd);
					register_write(socket_fd, epoll_fd);
					it->second.changeClientState(PROCESSING);
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				if (it->second.getClientState() == PROCESSING)
				{
					it->second.appendResponseBuffer("Message received\r\n");
					it->second.changeClientState(SENDING);
				}
				else if (it->second.getClientState() == SENDING)
				{
					write(socket_fd, it->second.getResponseBuffer().c_str(), it->second.getResponseLen());
					it->second.clearResponseBuffer();
					it->second.changeClientState(READING);
					delete_write(socket_fd, epoll_fd);
					register_read(socket_fd, epoll_fd);
				}
			}
		}
	}
	close(epoll_fd);
}