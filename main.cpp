#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "Client.hpp"

# define MAX_EVENTS SOMAXCONN

void	register_read(int socket, int kq)
{
	struct kevent	event;
	EV_SET(&event, socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(kq, &event, 1, NULL, 0, NULL);
}

void	register_write(int socket, int kq)
{
	struct kevent	event;
	EV_SET(&event, socket, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
	kevent(kq, &event, 1, NULL, 0, NULL);
}

void	delete_write(int socket, int kq)
{
	struct kevent	event;
	EV_SET(&event, socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	kevent(kq, &event, 1, NULL, 0, NULL);
}

void	delete_read(int socket, int kq)
{
	struct kevent	event;
	EV_SET(&event, socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(kq, &event, 1, NULL, 0, NULL);
}

int	main()
{
	std::vector<int>	monitored_sockets;
	std::vector<Client>	client_list;
	int	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	monitored_sockets.push_back(server_socket);
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
	register_read(server_socket, kq);
	while (true)
	{
		int nev = kevent(kq, NULL, 0, evList, MAX_EVENTS, NULL);
		for (int i = 0; i < nev; i++)
		{
			int socket_fd = evList[i].ident;
			if(evList[i].flags == EV_EOF)
			{
				for (std::vector<Client>::iterator it = client_list.begin(); it < client_list.end(); it++)
				{
					if (socket_fd == it->getClientSocket())
					{
						std::cout << "client disconnected." << std::endl;
						delete_read(it->getClientSocket(), kq);
						delete_write(it->getClientSocket(), kq);
						close(client_list[i].getClientSocket()); 
						client_list.erase(it);
						break ;
					}
				}
			}
			else if (evList[i].filter == EVFILT_READ)
			{
				if (socket_fd == server_socket)
				{
					Client	new_client(accept(server_socket, (struct sockaddr *)NULL, NULL));
					client_list.push_back(new_client);
					monitored_sockets.push_back(new_client.getClientSocket());
					std::cout << "New client connected." << std::endl;
					register_read(new_client.getClientSocket(), kq);
				}
				else
				{
					for (std::vector<Client>::iterator it = client_list.begin(); it < client_list.end(); it++)
					{
						if (socket_fd == it->getClientSocket() && it->getClientState() == READING)
						{
							char	message[1024];
							size_t	bytes_read = read(it->getClientSocket(), message, 1024);
							if (bytes_read <= 0)
							{
								std::cout << "client disconnected." << std::endl;
								delete_read(it->getClientSocket(), kq);
								delete_write(it->getClientSocket(), kq);
								close(it->getClientSocket()); 
								client_list.erase(it);
								break ;
							}
							it->appendRequestBuffer(message);
							std::cout << "Request from client: " << std::endl << it->getRequestBuffer() << std::endl;
							it->clearRequestBuffer();
							delete_read(it->getClientSocket(), kq);
							register_write(it->getClientSocket(), kq);
							it->changeClientState(PROCESSING);
						}
					}
				}
			}
			else if (evList[i].filter == EVFILT_WRITE)
			{
				for (std::vector<Client>::iterator it = client_list.begin(); it < client_list.end(); it++)
				{
					if (socket_fd != it->getClientSocket())
						continue;
					if (it->getClientState() == PROCESSING)
					{
						it->appendResponseBuffer("Message received\r\n");
						it->changeClientState(SENDING);
					}
					else if (it->getClientState() == SENDING)
					{
						write(socket_fd, it->getResponseBuffer().c_str(), it->getResponseLen());
						it->clearResponseBuffer();
						it->changeClientState(READING);
						delete_write(socket_fd, kq);
						register_read(socket_fd, kq);
					}
				}
			}
		}
	}
	close(kq);
}