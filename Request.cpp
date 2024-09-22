# include "Request.hpp"
# include <iostream>
# include <sstream>
# include <string>

/// *** Functions *** ///
#pragma region Functions

// test function!!
void Request::print_request() const {
	std::cout << "***************************************************" << std::endl;
	std::cout << "Method: " << method << std::endl;
	std::cout << "URI: " << uri << std::endl;
	std::cout << "HTTP Version: " << http_version << std::endl;
	std::cout << "Headers:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << "Body: " << body << std::endl;
	std::cout << "***************************************************" << std::endl;
}

bool	Request::parse_request_line(const std::string& line)
{
	std::istringstream line_stream(line);

	if (!(line_stream >> method >> uri >> http_version))
	{
		return false;
	}
	return true;
}

bool Request::parse_header(const std::string& line)
{
	std::string::size_type colon_pos;
	std::string header;
	std::string value;

	colon_pos = line.find(": ");
	if (colon_pos == std::string::npos)
		return false;
	header = line.substr(0, colon_pos);
	value = line.substr(colon_pos + 2);
	headers[header] = value;
	return true;
}

int		Request::parse_request(std::string request)
{
	std::istringstream request_stream(request);
	std::string line;

	if (!std::getline(request_stream, line) || !parse_request_line(line))
		return PARSE_INVALID_REQUEST_LINE;
	while (std::getline(request_stream, line) && line != "\r") {
		if (!parse_header(line)) {
			std::cerr << "Failed to parse header: " << line << std::endl;
			return PARSE_INVALID_HEADER;
		}
	}
	return PARSE_SUCCESS;
}

#pragma endregion
/// *** Destructor *** ///
#pragma region Destructor

Request::~Request()
{
	
}

#pragma endregion