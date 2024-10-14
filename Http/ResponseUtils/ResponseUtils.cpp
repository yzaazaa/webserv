# include "ResponseUtils.hpp"
# include "../Client.hpp"
# include "../../Server/Server.hpp"
#include <sys/event.h>
# include <sys/fcntl.h>

/// *** Constructors *** ///
#pragma region Constructors

ResponseUtils::ResponseUtils()
{

}

ResponseUtils::ResponseUtils(const ResponseUtils& other)
{
	*this = other;
}

#pragma endregion

/// *** Functions *** ///
#pragma region Functions

void	ResponseUtils::BadRequest400_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 400 Bad Request" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::URITooLong414_NoBody(Response& response)
{
	std::ostringstream	stream;
	
	stream << "HTTP/1.1 414 URI Too Long" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::NotImplemented501_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 501 Not Implemented" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::InternalServerError500_NoBody(Response& response, int kq, int client_socket)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 500 Internal Server Error" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
	KqueueUtils::EnableEvent(kq, client_socket, WRITE);
}

void	ResponseUtils::NotFound404_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 404 Not Found" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::MethodNotAllowed405_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 405 Method Not Allowed" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::MovedPermanently301_NoBody(Response& response, std::string const &location)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 301 Moved Permanently" << Endl_Request;
	stream << "Location: " << location << Endl_Request;
	stream << "Content-Type: text/plain" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::Forbidden403_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 403 Forbidden" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::Conflict409_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 409 Conflict" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::GatewayTimeout504_NoBody(Response& response, int kq, int client_socket)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 504 Gateway Timeout" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
	KqueueUtils::EnableEvent(kq, client_socket, WRITE);
}

void	ResponseUtils::NoContent204_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 204 No Content" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::OK200(Response& response, Client& client, int kq, int client_socket, Server &server)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 200 OK" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = false;
	int	fd = open(client.Request.uri.path.c_str(), O_RDONLY);
	if (fd == -1)
		return (ResponseUtils::InternalServerError500_NoBody(response, kq, client_socket), (void)0);
	KqueueUtils::RegisterEvents(kq, fd, true);
	server.addFd(fd, client_socket);
}

void	ResponseUtils::Created201(Response& response, Client& client, int kq, int client_socket, Server &server)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 201 Created" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	std::cout << response.Buffer << std::endl;
	response.IsLastResponse = true;
	response.CloseConnection = false;
	int	fd = open(client.Request.uri.path.c_str(), O_WRONLY);
	if (fd == -1)
		return (ResponseUtils::InternalServerError500_NoBody(response, kq, client_socket), (void)0);
	KqueueUtils::RegisterEvents(kq, fd);
	KqueueUtils::DisableEvent(kq, fd, READ);
	KqueueUtils::EnableEvent(kq, fd, WRITE);
	server.addFd(fd, client_socket);
}

#pragma endregion

/// *** Destructor *** ///
#pragma region Destructor

ResponseUtils::~ResponseUtils()
{

}

#pragma endregion