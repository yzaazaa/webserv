#pragma once
# include <iostream>
# include "../Response.hpp"
# include <sstream>

#define Endl_Request "\r\n"
#define DoubleEndl_Request "\r\n\r\n"

class ResponseUtils
{
	private:
		ResponseUtils();
		ResponseUtils(const ResponseUtils& other);

	public:
		// Functions
		//TODO: add a boolean if its fatal, to kill the connection or keep it alive (only stays alive if Connection is not close)
		static void	BadRequest400_NoBody(Response& response);
		static void	URITooLong414_NoBody(Response& response);
		static void	NotImplemented501_NoBody(Response& response);
		static void	InternalServerError500_NoBody(Response& response);

		static void	NotFound404_NoBody(Response& response);
		static void	MovedPermanently301_NoBody(Response& response, std::string& location);
		static void	MethodNotAllowed405_NoBody(Response& response);

		// Destructor
		~ResponseUtils();
};
