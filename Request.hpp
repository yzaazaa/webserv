#pragma once
# include <iostream>
# include <map>

enum ParseStatus {
    PARSE_SUCCESS,
    PARSE_INVALID_REQUEST_LINE,
    PARSE_INVALID_HEADER,
    PARSE_BODY_TOO_LARGE,
    PARSE_INCOMPLETE
};

class Request
{
	public:
		std::string method;
		std::string uri;
		std::string http_version;
		std::map<std::string ,std::string> headers;
		std::string body;


		int		parse_request(std::string request);
		bool	parse_request_line(const std::string& line);
		bool	parse_header(const std::string& line);
		void	print_request() const; // test function
		~Request();
	private:

};
