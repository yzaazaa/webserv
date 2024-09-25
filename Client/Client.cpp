#include "Client.hpp"
#include <sys/fcntl.h>
#include <sys/stat.h>

Client::Client() : request_buffer(""), response_buffer(""), client_state(READING), socket(-1), kq(-1) {}

Client::Client(int socket, int kq, int serverFd) : request_buffer(""), response_buffer(""), client_state(READING), socket(socket), kq(kq), serverFd(serverFd) {}

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
	this->serverFd = rhs.serverFd;
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

void 	Client::parseRequest()
{
	request.parse_request(request_buffer);
	request.print_request();
}

void	Client::handleRequest() {
	if (request.method == "GET") {
		// Simple response for GET request
		appendResponseBuffer("HTTP/1.1 200 OK\r\n");
		appendResponseBuffer("Content-Type: text/plain\r\n\r\n");
		appendResponseBuffer("Hello, World!\r\n");
	} else if (request.method == "POST") {
		appendResponseBuffer("HTTP/1.1 200 OK\r\n");
		appendResponseBuffer("Content-Type: text/plain\r\n\r\n");
		appendResponseBuffer("POST data received\r\n");
	} else if (request.method == "DELETE") {
		appendResponseBuffer("HTTP/1.1 200 OK\r\n");
		appendResponseBuffer("Content-Type: text/plain\r\n\r\n");
		appendResponseBuffer("Resource deleted\r\n");
	} else {
		// Unsupported method
		appendResponseBuffer("HTTP/1.1 405 Method Not Allowed\r\n");
		appendResponseBuffer("Allow: GET, POST, DELETE\r\n\r\n");
	}
}