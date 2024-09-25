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
		std::string	request_buffer;
		std::string	response_buffer;
		state		client_state;
		int			socket;
		int			kq;
		Request		request;

	public:
		Client();
		Client(int socket, int kq);
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
		void parseRequest() {
			request.parse_request(request_buffer);
			request.print_request();
  	 	}

    	void handleRequest() {
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
};