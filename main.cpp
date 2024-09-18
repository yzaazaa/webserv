#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include "Client.hpp"

# define MAX_EVENTS SOMAXCONN

void modify_event(int socket, int kq, short filter, u_short action) {
    struct kevent event;
    EV_SET(&event, socket, filter, action, 0, 0, NULL);
    kevent(kq, &event, 1, NULL, 0, NULL);
}

void	register_socket(int socket, int kq) {
	modify_event(socket, kq, EVFILT_READ, EV_ADD | EV_ENABLE);
	modify_event(socket, kq, EVFILT_WRITE, EV_ADD | EV_DISABLE);
}

void enable_write(int socket, int kq) {
    modify_event(socket, kq, EVFILT_WRITE, EV_ENABLE);
}

void enable_read(int socket, int kq) {
    modify_event(socket, kq, EVFILT_READ, EV_ENABLE);
}

void disable_write(int socket, int kq) {
    modify_event(socket, kq, EVFILT_WRITE, EV_DISABLE);
}

void disable_read(int socket, int kq) {
    modify_event(socket, kq, EVFILT_READ, EV_DISABLE);
}

void	delete_socket_events(int socket, int kq)
{
	modify_event(socket, kq, EVFILT_WRITE, EV_DELETE);
	modify_event(socket, kq, EVFILT_READ, EV_DELETE);
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
	struct kevent	evList[MAX_EVENTS];
	register_socket(server_socket, kq);
	while (true)
	{
		memset(evList, 0, sizeof(evList));
		int nev = kevent(kq, NULL, 0, evList, sizeof(evList), NULL);
		for (int i = 0; i < nev; i++)
		{
			int socket_fd = evList[i].ident;
			std::map<int, Client>::iterator it = client_map.find(socket_fd);
			if(evList[i].flags == EV_EOF)
			{
				std::cout << "client disconnected." << std::endl;
				delete_socket_events(socket_fd, kq);
				close(socket_fd); 
				client_map.erase(it);
			}
			else if (evList[i].filter == EVFILT_READ)
			{
				if (socket_fd == server_socket)
				{
					int	client_socket = accept(server_socket, (struct sockaddr *)NULL, NULL); 
					Client	new_client(client_socket);
					client_map[client_socket] = new_client;
					std::cout << "New client connected." << std::endl;
					register_socket(client_socket, kq);
				}
				else if (it->second.getClientState() == READING)
				{
					char	message[1024];
					size_t	bytes_read = read(socket_fd, message, 1024);
					if (bytes_read <= 0)
					{
						std::cout << "client disconnected." << std::endl;
						delete_socket_events(socket_fd, kq);
						close(socket_fd);
						client_map.erase(it);
						continue ;
					}
					it->second.appendRequestBuffer(message);
					std::cout << "Request from client: " << std::endl << it->second.getRequestBuffer() << std::endl;
					it->second.clearRequestBuffer();
					disable_read(socket_fd, kq);
					enable_write(socket_fd, kq);
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
				disable_write(socket_fd, kq);
				enable_read(socket_fd, kq);
			}
		}
	}
	close(kq);
}