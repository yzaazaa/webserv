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
# include "FileUtils.hpp"
# include "ResponseUtils/ResponseUtils.hpp"

/// *** Constructors *** ///
#pragma region Constructors

Client::Client() : ServerFd(-1), Location(NULL)
{

}
Client::Client(int serverFd) : ServerFd(serverFd), Location(NULL)
{

}

#pragma endregion

void	Client::methodDelete()
{
	if (FileUtils::pathNotFound(Request.uri.path))
		return (ResponseUtils::NotFound404_NoBody(this->Response), (void)0);
	if (FileUtils::isDirectory(Request.uri.path))
	{
		if (Request.uri.path.back() == '/')
		{
			// if (isCgi(Request.uri.path))
			// 	doCgiStuff();
			// else
			// {
					if (!FileUtils::deleteFolderContent(Request.uri.path))
					{
						if (FileUtils::hasWriteAccess(Request.uri.path))
							return (ResponseUtils::InternalServerError500_NoBody(this->Response), (void)0);
						return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
					}
					return (ResponseUtils::NoContent204_NoBody(this->Response), (void)0);
			// }
		}
		return (ResponseUtils::Conflict409_NoBody(this->Response), (void)0);
	}
	// if (isCgi(Request.uri.path))
	// 	doCgiStuff();
	// else
	// {
		if (std::remove(Request.uri.path.c_str()))
		{
			if (FileUtils::hasWriteAccess(Request.uri.path))
				return (ResponseUtils::InternalServerError500_NoBody(this->Response),(void)0);
			return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
		}
		return (ResponseUtils::NoContent204_NoBody(this->Response), (void)0);
	// }
}

bool	Client::getAutoIndex()
{
	return Location->IsAutoIndex.IsDefined && Location->IsAutoIndex.IsAutoIndexOn;
}

bool	Client::locationSupportUpload()
{
	return false;
}

bool	Client::isCgi()
{
	std::string	extension = ".py";
	int			extension_len = extension.length();
	int			uri_len = Request.uri.path.length();
	if (uri_len >= extension_len && !Request.uri.path.compare(uri_len - extension_len, extension_len, extension)
		&& Location->CgiConfig.IsDefined)
		return true;
	return false;
}

void	Client::methodGet(int kq, int socket, Server& server)
{
	if (FileUtils::pathNotFound(Request.uri.path))
		return (ResponseUtils::NotFound404_NoBody(this->Response), (void)0);
	if (FileUtils::isDirectory(Request.uri.path))
	{
		if (Request.uri.path.back() == '/')
		{
			if (FileUtils::dirHasIndexFiles(Request.uri.path))
			{
				// if (isCgi(Request.uri.path))
				// 	doCgiStuff();
				// else
				// {
						return (ResponseUtils::OK200(Response, *this, kq, socket, server), (void)0);
				// }
			}
			if (getAutoIndex()) // Auto index true if on
				return ; // 200 ok (return autoindex of directory (autoindex means loading page listing the directory entrys))
			return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
		}
		return (ResponseUtils::MovedPermanently301_NoBody(this->Response, Request.uri.path + "/"), (void)0);
	}
	if (FileUtils::hasReadAccess(Request.uri.path))
	{
		// if (isCgi(Request.uri.path))
		// 	doCgiStuff();
		// else
		// {
				return (ResponseUtils::OK200(Response, *this, kq, socket, server), (void)0);
		// }
	}
	return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
}

void	Client::methodPost(int kq, Server &server)
{
	(void)kq;
	(void)server;
	if (locationSupportUpload())
		return ; // 201 Created (and upload the post Request body)
	else
	{
		if (FileUtils::pathNotFound(Request.uri.path))
			return (ResponseUtils::NotFound404_NoBody(this->Response), (void)0);
		if (FileUtils::isDirectory(Request.uri.path))
		{
			if (Request.uri.path.back() == '/')
			{
				if (FileUtils::dirHasIndexFiles(Request.uri.path))
				{
					// if (isCgi(Request.uri.path))
					// 	doCgiStuff(); 
					// else
					// {
							return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
					// }
				}
				return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
			}
			return (ResponseUtils::MovedPermanently301_NoBody(this->Response, Request.uri.path + "/"), (void)0);
		}
		// if (isCgi(Request.uri.path))
		// 	doCgiStuff();
		// else
		// {
				return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
		// }
	}
}

void	Client::OnRequestCompleted(int kq, int socket, Server& server)
{
	if (Request.method == "get") {
		methodGet(kq, socket, server);
	} else if (Request.method == "post") {
		methodPost(kq, server);
	} else if (Request.method == "delete") {
		methodDelete();
	}
}

void	Client::OnSocket_ReadyForRead(Server& server, int kq, int fd)
{
	if (!Request.IsHeaderParsingDone)
	{
		if (!HeaderValidator::ReadAndParseHeader(*this, server, fd))
		{
			KqueueUtils::EnableWriting(kq, fd);
			return;
		}
		else if (Request.IsHeaderParsingDone)
		{
			HeaderValidator::RemoveHeaderFromBuffer(Request);
			// TEST_PREPARE_RESPONSE();
		}
		else
			return;
	}
	std::cout << "Body Parsing Not Yet Implemented" << std::endl;
	OnRequestCompleted(kq, fd, server);
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
	// Request_buffer.clear();

	if (Response.IsLastResponse)
	{
		if (Response.CloseConnection)
		{
			server.OnClientDisconnected(kq, fd); //TODO: this needs to be something like, OnFinished or something, to make sure it doesnt conflict with client diconnecting
			return;
		}
		// client has sent the last message for the given Request, reset the Request and wait for more
		Request.Reset();
	}

	Response.Reset();
	// Read more
	KqueueUtils::DisableEvent(kq, fd, WRITE);
	KqueueUtils::EnableEvent(kq, fd, READ);
}

void	Client::OnFile_ReadyForRead(int fd)
{
	(void)fd;
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

