#pragma once

#include <map>
#include <cstring>
#include <iostream>
#include <string>

#define PYTHON "/usr/local/bin/python3"
#define PHP "/usr/bin/php"
#define JAVA "/usr/bin/java"

class Client;

typedef std::map<std::string, std::string> stringMap;

class Cgi
{
	private:
		Client		&client;
		stringMap	envMap;
		std::string	scriptExecutor;
		char		*args[3];
		int			fd[2];
		int			stdFd[2];
		int			pid;

		void	getEnvMap(char **env);
		void	setEnv(char **env);
		void	fillExecArgs();
		char	**getEnvStrs();
		void	execFile(int kq);
		void	clean(char **env);

	public:
		Cgi(Client &client);
		~Cgi();

		void	run(char **env, int kq);
};