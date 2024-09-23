#include "Client.hpp"
#include <sys/fcntl.h>

Client::Client() : request_buffer(""), response_buffer(""), client_state(READING), socket(-1), read_event(0), write_event(0), kq(-1) {}

Client::Client(int socket, int kq) : request_buffer(""), response_buffer(""), client_state(READING), socket(socket), read_event(0), write_event(0), kq(kq)
{
	setNonBlock();
	registerClientSocket();
}

Client::Client(const Client &client)
{
    *this = client;
}

Client  &Client::operator=(const Client &rhs)
{
	this->client_state = rhs.client_state;
	this->request_buffer = rhs.request_buffer;
	this->response_buffer = rhs.response_buffer;
	this->socket = rhs.socket;
	this->kq = rhs.kq;
	return (*this);
}

Client::~Client() {}

int		Client::getClientSocket(void)
{
	return this->socket;
}

void		Client::changeClientState(state state)
{
	this->client_state = state;
	if (state == PROCESSING)
	{
		this->disableReadEvent();
		this->enableWriteEvent();
	}
	else if (state == READING)
	{
		this->disableWriteEvent();
		this->enableReadEvent();
	}
}

state	&Client::getClientState()
{
	return this->client_state;
}

void	Client::appendRequestBuffer(char *msg)
{
	std::string	message(msg);
	this->request_buffer += message;
}

void	Client::appendResponseBuffer(std::string msg)
{
	this->response_buffer += msg;
}

std::string	&Client::getRequestBuffer()
{
	return this->request_buffer;
}

std::string	&Client::getResponseBuffer()
{
	return this->response_buffer;
}

int			Client::getResponseLen()
{
	return this->response_buffer.length();
}

void	Client::clearRequestBuffer()
{
	this->request_buffer.clear();
}

void	Client::clearResponseBuffer()
{
	this->response_buffer.clear();
}

void	Client::modifyEvent(short filter, u_short action)
{
	struct kevent event;
    EV_SET(&event, this->socket, filter, action, 0, 0, NULL);
    kevent(this->kq, &event, 1, NULL, 0, NULL);
}

void	Client::registerClientSocket()
{
	modifyEvent(EVFILT_READ, EV_ADD | EV_ENABLE);
	this->read_event = true;
	modifyEvent(EVFILT_WRITE, EV_ADD | EV_DISABLE);
}

void	Client::enableWriteEvent()
{
	modifyEvent(EVFILT_WRITE, EV_ENABLE);
	this->write_event = true;
}

void	Client::enableReadEvent()
{
	modifyEvent(EVFILT_READ, EV_ENABLE);
	this->read_event = true;
}

void	Client::disableReadEvent()
{
	modifyEvent(EVFILT_READ, EV_DISABLE);
	this->read_event = false;
}

void	Client::disableWriteEvent()
{
	modifyEvent(EVFILT_WRITE, EV_DISABLE);
	this->write_event = false;
}

void	Client::deleteClientEvents()
{
	modifyEvent(EVFILT_WRITE, EV_DELETE);
	this->write_event = false;
	this->read_event = false;
	modifyEvent(EVFILT_READ, EV_DELETE);
}

bool	Client::getReadEvent()
{
	return read_event;
}

bool	Client::getWriteEvent()
{
	return write_event;
}

void	Client::disconnect()
{
	deleteClientEvents();
	close(socket); 
}

void	Client::setNonBlock()
{
	int	flags = fcntl(socket, F_GETFL, 0);
	if (flags == -1)
	{
		std::cerr << "Failed to get fd flags." << std::endl;
		return ;
	}
	if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		std::cerr << "Failed to set fd to non blocking mode." << std::endl;
		return ;
	}
}

void	Client::readRequest(Server &server)
{
	char	message[1024];
	size_t	bytes_read = read(socket, message, 1024);
	if (bytes_read <= 0)
		server.disconnectClient(*this);
	appendRequestBuffer(message);
	// std::cout << "Request from client: " << std::endl << request_buffer << std::endl;
}