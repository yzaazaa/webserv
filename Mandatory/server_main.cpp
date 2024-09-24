#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <string.h>
#include <unistd.h>

int	createSocket(int domain, int type, int protocol)
{
	// Create Socket
	int server_socketfd = socket(domain, type, protocol);

	if (server_socketfd == -1)
		return (perror("Socket Opening"), server_socketfd);
	
	// Speed up restarting (allows using the same port/adress by multiple sockets)
	int opt = 1;
	if (setsockopt(server_socketfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof opt) < 0)
		return (perror("setSocketopt"), -1);

	return (server_socketfd);
}

int serverBindAndListen(int serverSocketfd, int sin_fam, int sin_port, int addr)
{
	sockaddr_in	serverAddress;
	bzero((char *)&serverAddress, sizeof(serverAddress)); //maybe not needed
	serverAddress.sin_family = sin_fam;
	serverAddress.sin_port = htons(sin_port);
	serverAddress.sin_addr.s_addr = addr;

	if (bind(serverSocketfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		return (perror("Socket Binding"), -1);
	if (listen(serverSocketfd, 5) == -1)
		return (perror("Socket listening"), -1);
	return (1);
}

int setServerSocketNonBlocking(int serverSocketfd)
{
	int flags = fcntl(serverSocketfd, F_GETFL, 0);
    if (flags == -1)
        return (perror("fcntl"), -1);
    if (fcntl(serverSocketfd, F_SETFL, flags | O_NONBLOCK) == -1)
        return (perror("fcntl"), -1);
	return (1);
}

int serverSetup(int &server_socketfd)
{
	server_socketfd = createSocket(AF_INET, SOCK_STREAM, 0);
	if (server_socketfd == -1)
		return (-1);

	if (serverBindAndListen(server_socketfd, AF_INET, 1234, INADDR_ANY) == -1)
		return (-1);

	if (setServerSocketNonBlocking(server_socketfd) == -1)
		return (-1);
	return (1);
}


int main()
{
	int	server_socketfd = -1;
	if (serverSetup(server_socketfd) == -1)
		return (EXIT_FAILURE);


	// Keq setup

	// Prepare the kqueue.
	int kq = kqueue();

	// Create event 'filter', these are the events we want to monitor.
    // Here we want to monitor: socket_listen_fd, for the events: EVFILT_READ 
    // (when there is data to be read on the socket), and perform the following
    // actions on this kevent: EV_ADD and EV_ENABLE (add the event to the kqueue 
    // and enable it).
	struct kevent change_event[4];
    EV_SET(change_event, server_socketfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

	// Register kevent with the kqueue.
    if (kevent(kq, change_event, 1, NULL, 0, NULL) == -1)
    {
        perror("kevent");
        exit(1);
    }
	std::cout << "kevent registered with kqueue" << std::endl;

	while (true)
	{
		// Check for new events, but do not register new events with
        // the kqueue. Hence the 2nd and 3rd arguments are NULL, 0.
        // Only handle 1 new event per iteration in the loop; 5th
        // argument is 1.

		struct kevent event[4];
        int new_events = kevent(kq, NULL, 0, event, 1, NULL);
        if (new_events == -1)
        {
            perror("kevent");
            exit(1);
        }
		std::cout << std::endl << "something happend" << std::endl;
		for (int i = 0; new_events > i; i++)
		{
			int event_fd = event[i].ident;

            // When the client disconnects an EOF is sent. By closing the file
            // descriptor the event is automatically removed from the kqueue.
            if (event[i].flags & EV_EOF)
            {
				std::cout << "Client has disconnected" << std::endl;
                close(event_fd);
				continue;
            }

			// If the new event's file descriptor is the same as the listening
            // socket's file descriptor, we are sure that a new client wants 
            // to connect to our socket.
            if (event_fd == server_socketfd)
            {
                // Incoming socket connection on the listening socket.
                // Create a new socket for the actual connection to client.
				struct sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);
                int socket_connection_fd = accept(event_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
                if (socket_connection_fd == -1)
                    perror("Accept socket error");

                // Put this new socket connection also as a 'filter' event
                // to watch in kqueue, so we can now watch for events on this
                // new socket.
                EV_SET(change_event, socket_connection_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                if (kevent(kq, change_event, 1, NULL, 0, NULL) < 0)
                    perror("kevent error");

				std::cout << "New client Connected, his FD: " <<  socket_connection_fd << std::endl;
				continue;
            }

			if (event[i].filter & EVFILT_READ)
            {
                // Read bytes from socket
                char buf[1024];
                size_t bytes_read = recv(event_fd, buf, sizeof(buf), 0);
				std::cout << "FD: " << event_fd << " Read: " << bytes_read << " bytes, String: " << buf;
				continue;
            }


			std::cout << "Non of the above" << std::endl;
		}

	}


	close(server_socketfd);
	return (EXIT_SUCCESS);
	
}