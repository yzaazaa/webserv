#pragma once

#include <sys/event.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <fcntl.h>
#include "../Server/Server.hpp"
#include "../Request/Request.hpp"
#include <sys/stat.h>

enum state
{
	READING,
	PROCESSING,
	SENDING,
	CLOSED
};

class	Client
{
	private:
		ServerInstance* instance;
		std::string	request_buffer;
		std::string	response_buffer;
		state		client_state;
		int			socket;
		int			kq;
		Request		request;
		int			serverFd;

	public:
		Client();
		Client(int socket, int kq, int serverFd);
		Client(const Client &client);
		Client	&operator=(const Client &rhs);
		~Client();

		void		changeClientState(state state);
		state		&getClientState();
		void		appendRequestBuffer(char *msg);
		void		appendResponseBuffer(std::string msg);
		std::string	&getRequestBuffer();
		std::string	&getResponseBuffer();
		int			getResponseLen();
		void		clearRequestBuffer();
		void		clearResponseBuffer();
		void		readRequest(Server &server);
		void 		parseRequest();
    	void		handleRequest();
};