# include "Client.hpp"
#include "HeaderValidator/HeaderValidator.hpp"
#include <ostream>
# include <stdexcept>
# include <sys/fcntl.h>
#include <sys/socket.h>
# include <sys/stat.h>
# include <sstream>
# include <string>
# include <cstddef>
# include <iostream>
# include "FileUtils.hpp"
# include "ResponseUtils/ResponseUtils.hpp"

/// *** Constructors *** ///
#pragma region Constructors

Client::Client(int serverFd, int socket) : ServerFd(serverFd), Location(NULL), socket(socket), lastTime(ft_time())
{

}

#pragma endregion

void	Client::methodDelete(int kq, int socket)
{
	if (FileUtils::pathNotFound(Request.uri.path))
		return (ResponseUtils::NotFound404_NoBody(this->Response), (void)0);
	if (FileUtils::isDirectory(Request.uri.path))
	{
		if (Request.uri.path.back() == '/')
		{
			if (!FileUtils::deleteFolderContent(Request.uri.path))
			{
				if (FileUtils::hasWriteAccess(Request.uri.path))
					return (ResponseUtils::InternalServerError500_NoBody(this->Response, kq, socket), (void)0);
				return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
			}
			return (ResponseUtils::NoContent204_NoBody(this->Response), (void)0);
		}
		return (ResponseUtils::Conflict409_NoBody(this->Response), (void)0);
	}
	if (std::remove(Request.uri.path.c_str()))
	{
		if (FileUtils::hasWriteAccess(Request.uri.path))
			return (ResponseUtils::InternalServerError500_NoBody(this->Response, kq, socket),(void)0);
		return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
	}
	return (ResponseUtils::NoContent204_NoBody(this->Response), (void)0);
}

bool	Client::getAutoIndex()
{
	return Location->IsAutoIndex.IsDefined && Location->IsAutoIndex.IsAutoIndexOn;
}

bool	Client::locationSupportUpload()
{
	return false;
}

bool Client::isCgi()
{
    std::vector<std::string> extensions;
    extensions.push_back(".py");
    extensions.push_back(".php");
    extensions.push_back(".java");
    int uri_len = Request.uri.path.length();
    for (size_t i = 0; i < extensions.size(); i++)
	{
		std::string	&extension = extensions[i];
        int extension_len = extension.length();
        if (uri_len >= extension_len && 
            !Request.uri.path.compare(uri_len - extension_len, extension_len, extension))
		{
			Request.uri.extension = extension;
            return true;
		}
    }
    return false;
}

void	Client::methodGet(int kq, int socket, Server& server)
{
	std::cout << "in get\n";
	if (FileUtils::pathNotFound(Request.uri.path))
	{
		return (ResponseUtils::NotFound404_NoBody(this->Response), (void)0);
	}
	if (FileUtils::isDirectory(Request.uri.path))
	{
		if (Request.uri.path.back() == '/')
		{
			if (FileUtils::dirHasIndexFiles(Request.uri.path))
			{
				if (isCgi())
					return Cgi.run(server.getEnv(), kq, server);
				else
					return (ResponseUtils::OK200(Response, *this, kq, socket, server), (void)0);
			}
			if (getAutoIndex()) // Auto index true if on
				return (ResponseUtils::OK200(Response, *this, kq, socket, server), (void)0);
			return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
		}
		return (ResponseUtils::MovedPermanently301_NoBody(this->Response, Request.uri.path + "/"), (void)0);
	}
	if (FileUtils::hasReadAccess(Request.uri.path))
	{
		if (isCgi())
			return Cgi.run(server.getEnv(), kq, server);
		else
			return (ResponseUtils::OK200(Response, *this, kq, socket, server), (void)0);
	}
	return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
}

void	Client::methodPost(int kq, int socket, Server &server)
{
	if (locationSupportUpload())
		return (ResponseUtils::Created201(Response, *this, kq, socket, server), (void)0);
	if (FileUtils::pathNotFound(Request.uri.path))
		return (ResponseUtils::NotFound404_NoBody(this->Response), (void)0);
	if (FileUtils::isDirectory(Request.uri.path))
	{
		if (Request.uri.path.back() == '/')
		{
			if (FileUtils::dirHasIndexFiles(Request.uri.path))
			{
				if (isCgi())
					return Cgi.run(server.getEnv(), kq, server); 
				else
					return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
			}
			return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
		}
		return (ResponseUtils::MovedPermanently301_NoBody(this->Response, Request.uri.path + "/"), (void)0);
	}
	if (isCgi())
		return Cgi.run(server.getEnv(), kq, server);
	else
		return (ResponseUtils::Forbidden403_NoBody(this->Response), (void)0);
}

void	Client::OnRequestCompleted(int kq, int socket, Server& server)
{
	Cgi.setClient(this);
	if (Request.method == "get") {
		methodGet(kq, socket, server);
	} else if (Request.method == "post") {
		methodPost(kq, socket, server);
	} else if (Request.method == "delete") {
		methodDelete(kq, socket);
	}
	lastTime = ft_time();
}

void	Client::OnSocket_ReadyForRead(Server& server, int kq, int fd)
{
	if (!Request.IsHeaderParsingDone)
	{
		if (!HeaderValidator::ReadAndParseHeader(*this, server, kq, fd))
		{
			KqueueUtils::EnableWriting(kq, fd);
			lastTime = ft_time();
			return;
		}
		else if (Request.IsHeaderParsingDone)
		{
			HeaderValidator::RemoveHeaderFromBuffer(Request);
			// TEST_PREPARE_RESPONSE();
		}
		else
		{
			lastTime = ft_time();
			return;
		}
	}
	std::cout << "Body Parsing Not Yet Implemented" << std::endl;
	Request.uri.path.erase(0,1);
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
	lastTime = ft_time();
	// Read more
	KqueueUtils::DisableEvent(kq, fd, WRITE);
	KqueueUtils::EnableEvent(kq, fd, READ);
}

void	Client::OnFile_ReadyForRead(Server &server, int kq, int fd)
{
	std::cout << fd << " ready ro read\n";
	char	message[READING_BUFFER_SIZE + 1];
	int	bytes_read = read(fd, message, READING_BUFFER_SIZE);
	lastTime = ft_time();
	if (bytes_read < 0)
	{
		if (isCgi() && Cgi.finished)
		{
			KqueueUtils::DeleteEvents(kq, fd);
			server.eraseFd(fd);
			Cgi.clean(server, kq);
			ResponseUtils::InternalServerError500_NoBody(Response, kq, socket);
			lastTime = ft_time();
			return ;
		}
	}
	else if (bytes_read == 0)
	{
		KqueueUtils::DisableEvent(kq, fd, READ);
		if (isCgi() && Cgi.finished)
			return Cgi.prepareResponse(server, kq, fd);
	}
	message[bytes_read] = '\0';
	std::cout << "\n----------------------------------------\n" << message << "\n----------------------------------------\n";
	if (Response.Buffer.empty())
	{
		std::ostringstream	stream;
		if (Request.method == "get")
			stream << "HTTP/1.1 200 OK" << Endl_Request;
		else
			stream << "HTTP/1.1 201 CREATED" << Endl_Request;

		Response.Buffer = stream.str();
	}
	Response.Buffer += message;
	if (bytes_read < READING_BUFFER_SIZE)
	{
		KqueueUtils::DisableEvent(kq, fd, READ);
		if (isCgi() && Cgi.finished)
			return Cgi.prepareResponse(server, kq, fd);
	}
}

void	Client::OnFile_ReadyForWrite(Server& server, int kq, int fd)
{
	if (isCgi())
	{
		KqueueUtils::DisableEvent(kq, fd, WRITE);
		if (write(fd, Request.body.c_str(), Request.body.length()) == -1)
		{
			std::cerr << "error wrintg\n";
			Cgi.clean(server, kq);
			lastTime = ft_time();
			return (ResponseUtils::InternalServerError500_NoBody(Response, kq, socket), (void)0);
		}
		std::cerr << "Wrote to file successfully\n";
		Cgi.execFile(kq, server);
	}
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

