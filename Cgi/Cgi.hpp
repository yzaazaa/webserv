#pragma once

#include <map>
#include <cstring>
#include <iostream>
#include <string>
#include <signal.h>

#define PYTHON "/usr/local/bin/python3"
#define PHP "/usr/bin/php"
#define JAVA "/usr/bin/java"

class Client;
class Server;

typedef std::map<std::string, std::string> stringMap;

class Cgi
{
	private:
		Client		&client;
		stringMap	envMap;
		char		**_env;
		std::string	scriptExecutor;
		char		*args[3];
		int			fd[2];
		int			stdFd[2];
		int			pid;
		bool		ready;

		void	getEnvMap(char **env);
		void	setEnv(char **env);
		void	fillExecArgs();
		char	**getEnvStrs();
		void	prepareFds(Server& server, int kq);

	public:
		Cgi(Client &client);
		~Cgi();

		void	run(char **env, int kq, Server &server);
		void	execFile(int kq, Server &server);
		void	prepareResponse(Server& server, int kq, int fd);
		void	clean();
		bool	getStatus();
};