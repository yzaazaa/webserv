#include "Client.hpp"

Client::Client() : request_buffer(""), response_buffer(""), client_state(READING), socket(-1) {}

Client::Client(int socket) : request_buffer(""), response_buffer(""), client_state(READING), socket(socket) {}

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
	return (*this);
}

Client::~Client() {}

void	Client::setClientSocket(int socket)
{
	this->socket = socket;
}

int		Client::getClientSocket(void)
{
	return this->socket;
}

void		Client::changeClientState(state state)
{
	this->client_state = state;
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

void	Client::appendResponseBuffer(char *msg)
{
	std::string	message(msg);
	this->response_buffer += message;
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
	return (this->response_buffer).length();
}

void	Client::clearRequestBuffer()
{
	this->request_buffer = "";
}

void	Client::clearResponseBuffer()
{
	this->response_buffer = "";
}
