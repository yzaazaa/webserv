# include "ResponseUtils.hpp"

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
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::URITooLong414_NoBody(Response& response)
{
	std::ostringstream	stream;
	
	stream << "HTTP/1.1 414 URI Too Long" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::NotImplemented501_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 501 Not Implemented" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::InternalServerError500_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 500 Internal Server Error" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::NotFound404_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 404 Not Found" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::MethodNotAllowed405_NoBody(Response& response)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 405 Method Not Allowed" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	response.IsLastResponse = true;
	response.CloseConnection = true;
}

void	ResponseUtils::MovedPermanently301_NoBody(Response& response, std::string& location)
{
	std::ostringstream	stream;

	stream << "HTTP/1.1 301 Moved Permanently" << Endl_Request;
	stream << "Location: " << location << Endl_Request;
	stream << "Content-Type: text/plain" << Endl_Request;
	stream << "Content-Length: 0" << DoubleEndl_Request;

	response.Buffer = stream.str();
	response.IsLastResponse = true;
	response.CloseConnection = true;
}


#pragma endregion

/// *** Destructor *** ///
#pragma region Destructor

ResponseUtils::~ResponseUtils()
{

}

#pragma endregion