#pragma once

# include <sys/event.h>
# include <iostream>
# include <unistd.h>
# include <sstream>
# include <fcntl.h>
# include "../Server/Server.hpp"
# include "Request.hpp"
# include <sys/stat.h>
# include "Response.hpp"
# include "HeaderValidator/HeaderValidator.hpp"

#define READING_BUFFER_SIZE 1024

class	Client
{
	public:
		Client();
		Client(int server);

		~Client();
	
		Response			Response;
		Request				Request;
		int					ServerFd;
		LocationInstance*	Location;

		
		void	OnSocket_ReadyForRead(Server& server, int kq, int fd);
		// void	OnFile_ReadyForRead(int fd);

		void	OnSocket_ReadyForWrite(Server& server, int kq, int fd);
		// void	OnFile_ReadyForWrite(int fd);

		void	OnRequestCompleted();
		bool	getAutoIndex();
		bool	locationSupportUpload();
		bool	isCgi();
		void	methodGet();
		void	methodPost();
		void	methodDelete();
		// void	OnDisconnect();

		/// *** DEBUG *** ///
    	void		TEST_PREPARE_RESPONSE();
};

