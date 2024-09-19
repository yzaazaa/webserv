#pragma once

#include <sys/event.h>
#include <iostream>
#include <unistd.h>

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
		bool		read_event;
		bool		write_event;
		int			kq;

		void		modifyEvent(short filter, u_short action);
		void		registerClientSocket();
	public:
		Client();
		Client(int socket, int kq);
		Client(const Client &client);
		Client	&operator=(const Client &rhs);
		~Client();

		void		setClientSocket(int socket);
		int			getClientSocket();
		void		changeClientState(state state);
		state		&getClientState();
		void		appendRequestBuffer(char *msg);
		void		appendResponseBuffer(std::string msg);
		std::string	&getRequestBuffer();
		std::string	&getResponseBuffer();
		int			getResponseLen();
		void		clearRequestBuffer();
		void		clearResponseBuffer();
		void		enableWriteEvent();
		void		enableReadEvent();
		void		disableWriteEvent();
		void		disableReadEvent();
		void		deleteClientEvents();
		bool		getReadEvent();
		bool		getWriteEvent();
		void		disconnect();
};