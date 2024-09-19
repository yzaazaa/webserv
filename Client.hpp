#pragma once

#include <sys/event.h>
#include <iostream>
#include <unistd.h>
#include <sstream>

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
		std::string method;
    	std::string path;

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
		void parseRequest() {
			std::istringstream request_stream(request_buffer);
			std::string request_line;
			std::getline(request_stream, request_line);
			std::istringstream line_stream(request_line);

			// Parse method and path
			line_stream >> method >> path;

			// Trim any trailing newlines or whitespace from path
			path.erase(std::remove(path.begin(), path.end(), '\r'), path.end());
			path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());

			std::cout << "Parsed request: Method=" << method << " Path=" << path << std::endl;
  	 	}

    	void handleRequest() {
			if (method == "GET") {
				// Simple response for GET request
				appendResponseBuffer("HTTP/1.1 200 OK\r\n");
				appendResponseBuffer("Content-Type: text/plain\r\n\r\n");
				appendResponseBuffer("Hello, World!\r\n");
			} else if (method == "POST") {
				appendResponseBuffer("HTTP/1.1 200 OK\r\n");
				appendResponseBuffer("Content-Type: text/plain\r\n\r\n");
				appendResponseBuffer("POST data received\r\n");
			} else if (method == "DELETE") {
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