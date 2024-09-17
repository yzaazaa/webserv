#pragma once

#include <iostream>

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
		std::string	request_buffer;
		std::string	response_buffer;
		state		client_state;
		int			socket;
	public:
		Client();
		Client(int socket);
		Client(const Client &client);
		Client	&operator=(const Client &rhs);
		~Client();

		void		setClientSocket(int socket);
		int			getClientSocket();
		void		changeClientState(state state);
		state		&getClientState();
		void		appendRequestBuffer(char *msg);
		void		appendResponseBuffer(char *msg);
		std::string	&getRequestBuffer();
		std::string	&getResponseBuffer();
		int			getResponseLen();
		void		clearRequestBuffer();
		void		clearResponseBuffer();
};