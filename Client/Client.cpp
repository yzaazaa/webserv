#include "Client.hpp"
#include <sys/fcntl.h>
#include <sys/stat.h>

Client::Client() : request_buffer(""), response_buffer(""), client_state(READING), socket(-1), kq(-1) {}

Client::Client(int socket, int kq) : request_buffer(""), response_buffer(""), client_state(READING), socket(socket), kq(kq) {}

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

void		Client::changeClientState(state state)
{
	this->client_state = state;
	if (state == PROCESSING)
	{
		KqueueUtils::DisableEvent(kq, socket, READ);
		KqueueUtils::EnableEvent(kq, socket, WRITE);
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

void	Client::readRequest(Server &server)
{
	char	message[1024];
	size_t	bytes_read = read(socket, message, 1024);
	if (bytes_read <= 0)
		server.OnClientDisconnected(kq, socket);
	appendRequestBuffer(message);
}
