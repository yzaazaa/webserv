# include "Client.hpp"
#include "HeaderValidator/HeaderValidator.hpp"
#include <ostream>
# include <stdexcept>
# include <sys/fcntl.h>
# include <sys/stat.h>
# include <sstream>
# include <string>
# include <cstddef>
# include <iostream>

/// *** Constructors *** ///
#pragma region Constructors

Client::Client() : ServerFd(-1), Location(NULL)
{

}
Client::Client(int serverFd) : ServerFd(serverFd), Location(NULL)
{

}

#pragma endregion

void	Client::OnSocket_ReadyForRead(Server& server, int kq, int fd)
{
	if (!Request.IsHeaderParsingDone)
	{
		if (!HeaderValidator::ReadAndParseHeader(*this, server, fd))
			KqueueUtils::EnableWriting(kq, fd);
		else if (Request.IsHeaderParsingDone)
		{
			HeaderValidator::RemoveHeaderFromBuffer(Request);
			KqueueUtils::EnableWriting(kq, fd); //TODO: remove this and when functions methds are implemented
			TEST_PREPARE_RESPONSE();
		}
		return;
	}
	std::cout << "Body Parsing Not Yet Implemented" << std::endl;
}

void	Client::OnSocket_ReadyForWrite(Server& server, int kq, int fd)
{
	(void)server;
	(void)kq;
	(void)fd;

	std::cout << "Writing to client: --------------------------" << std::endl;
	std::cout << Response.Buffer << std::endl;
	std::cout << "Last?: " << (Response.IsLastResponse ? "True" : "False") << std::endl;
	std::cout << "CloseConn?: " << (Response.CloseConnection ? "True" : "False") << std::endl;
	std::cout << "End: --------------------------" << std::endl;

	write(fd, Response.Buffer.c_str(), Response.Buffer.length());
	Response.Buffer.clear();
	// request_buffer.clear();

	if (Response.IsLastResponse)
	{
		if (Response.CloseConnection)
		{
			server.OnClientDisconnected(kq, fd); //TODO: this needs to be something like, OnFinished or something, to make sure it doesnt conflict with client diconnecting
			return;
		}
		// client has sent the last message for the given request, reset the request and wait for more
		Request.Reset();
	}

	Response.Reset();
	// Read more
	KqueueUtils::DisableEvent(kq, fd, WRITE);
	KqueueUtils::EnableEvent(kq, fd, READ);
}

void	Client::TEST_PREPARE_RESPONSE()
{
	std::string body;
	Response.IsLastResponse = true;
	Response.CloseConnection = false;

	if (Request.method == "get")
	{
		body += "<!DOCTYPE html>\r\n";
		body += "<html lang=\"en\">\r\n";
		body += "<head>\r\n";
		body += "<meta charset=\"UTF-8\">\r\n";
		body += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n";
		body += "<title>Simple Form</title>\r\n";
		body += "</head>\r\n";
		body += "<body>\r\n";
		body += "<h1>Submit Your Details</h1>\r\n";
		body += "<form action=\"/submit-form\" method=\"POST\">\r\n";
		body += "<label for=\"name\">Name:</label><br>\r\n";
		body += "<input type=\"text\" id=\"name\" name=\"name\"><br><br>\r\n";
		body += "<label for=\"email\">Email:</label><br>\r\n";
		body += "<input type=\"email\" id=\"email\" name=\"email\"><br><br>\r\n";
		body += "<input type=\"submit\" value=\"Submit\">\r\n";
		body += "</form>\r\n";
		body += "</body>\r\n";
		body += "</html>\r\n";

		Response.Buffer += "HTTP/1.1 200 OK\r\n";
		Response.Buffer += "Content-Length: " + std::to_string(body.size()) + "\r\n";
		Response.Buffer += "Content-Type: text/html\r\n\r\n";
		Response.Buffer += body;
	}
	else if (Request.method == "post")
	{
		Response.Buffer += "HTTP/1.1 200 OK\r\n";
		Response.Buffer += "Content-Type: text/plain\r\n\r\n";
		Response.Buffer += "POST data received\r\n";
	}
	else if (Request.method == "delete")
	{
		Response.Buffer += "HTTP/1.1 200 OK\r\n";
		Response.Buffer += "Content-Type: text/plain\r\n\r\n";
		Response.Buffer += "Resource deleted\r\n";
	}
	else
	{
		// Unsupported method
		Response.Buffer += "HTTP/1.1 405 Method Not Allowed\r\n";
		Response.Buffer += "Allow: GET, POST, DELETE\r\n\r\n";
	}
}

/// *** Destructor *** ///
#pragma region Destructor

Client::~Client() {}

#pragma endregion

