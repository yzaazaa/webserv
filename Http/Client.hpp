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
# include "../Cgi/Cgi.hpp"

#define READING_BUFFER_SIZE 1024

class	Client
{
	public:
		Client(int server, int socket);

		~Client();
	
		Response			Response;
		Request				Request;
		int					ServerFd;
		LocationInstance*	Location;
		Cgi					Cgi;
		int					socket;
		long				lastTime;
		
		void	OnSocket_ReadyForRead(Server& server, int kq, int fd);
		void	OnFile_ReadyForRead(Server& server, int kq, int fd);

		void	OnSocket_ReadyForWrite(Server& server, int kq, int fd);
		void	OnFile_ReadyForWrite(Server& server, int kq, int fd);

		void	OnRequestCompleted(int kq, int socket, Server& server);
		bool	getAutoIndex();
		bool	locationSupportUpload();
		bool	isCgi();
		void	methodGet(int kq, int socekt, Server& server);
		void	methodPost(int kq, int socket, Server& server);
		void	methodDelete(int kq, int socket);
		// void	OnDisconnect();

		/// *** DEBUG *** ///
    	void		TEST_PREPARE_RESPONSE();
};

