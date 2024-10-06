#include "Cgi.hpp"
#include <string>
#include <unistd.h>
#include <utility>
#include "../Http/Client.hpp"

Cgi::Cgi(Client &client) : client(client) {}

Cgi::~Cgi() {}

void	Cgi::getEnvMap(char **env)
{
	for (int i = 0; env[i] != NULL; i++)
	{
		char	*equal = std::strchr(env[i], '=');
		if (equal)
		{
			std::string key(env[i], env[i] - equal);
			std::string val(equal + 1);
			envMap.insert(std::make_pair(key, val));
		}
		else
		{
			std::string	key(env[i]);
			envMap.insert(std::make_pair(key, ""));
		}
	}
}

void	Cgi::setEnv(char **env)
{
	getEnvMap(env);
	envMap.insert(std::make_pair("SERVER_NAME", client.Request.headers["host"]));
	envMap.insert(std::make_pair("GATEWAY_INTERFACE", "CGI/1.1"));
	envMap.insert(std::make_pair("SERVER_PROTOCOL", "HTTP/1.1"));
	envMap.insert(std::make_pair("SERVER_PORT", "8090"));
	envMap.insert(std::make_pair("REMOTE_ADDR", "0.0.0.0"));
	envMap.insert(std::make_pair("REQUEST_METHOD", client.Request.method));
	envMap.insert(std::make_pair("SCRIPT_NAME", client.Request.uri.path));
	envMap.insert(std::make_pair("QUERY_STRING", client.Request.uri.query));
	envMap.insert(std::make_pair("CONTENT_TYPE", client.Request.headers["content-type"].substr(0, client.Request.headers["content-type"].find(';'))));
	envMap.insert(std::make_pair("CONTENT_LENGTH", std::to_string(client.Request.body.length())));
}

void	Cgi::fillExecArgs()
{
	if (client.Request.uri.extension == ".py")
		scriptExecutor = PYTHON;
	else if (client.Request.uri.extension == ".java")
		scriptExecutor = JAVA;
	else
		scriptExecutor = PHP;
	args[0] = const_cast<char *>(scriptExecutor.c_str());
	args[1] = const_cast<char *>(client.Request.uri.path.c_str());
	args[2] = NULL;
}

char	**Cgi::getEnvStrs()
{
	char** envArray = new char*[envMap.size() + 1];
    
    size_t index = 0;
    for (std::map<std::string, std::string>::const_iterator it = envMap.begin(); it != envMap.end(); ++it) {
        const std::string& key = it->first;
        const std::string& value = it->second;
        std::string keyValue = key + "=" + value;
        envArray[index] = new char[keyValue.length() + 1];
        std::strcpy(envArray[index], keyValue.c_str());
        ++index;
    }
    envArray[index] = NULL;
    return envArray;
}

void	Cgi::clean(char **env)
{
	dup2(stdFd[0], STDIN_FILENO);
	dup2(stdFd[1], STDOUT_FILENO);
	close(fd[0]);
	close(fd[1]);
	close(stdFd[0]);
	close(stdFd[1]);
	if (env)
	{
		for (int i = 0; env[i] != NULL; i++)
			delete[] env[i];
		delete[] env;
	}
}

void	Cgi::execFile(int kq)
{
	if (pipe(fd) == -1)
		return (ResponseUtils::InternalServerError500_NoBody(client.Response), (void)0);
	if ((stdFd[0] = dup(STDIN_FILENO)) == -1)
		return (ResponseUtils::InternalServerError500_NoBody(client.Response), (void)0);
	if ((stdFd[1] = dup(STDOUT_FILENO)) == -1)
		return (ResponseUtils::InternalServerError500_NoBody(client.Response), (void)0);
	char	**env = getEnvStrs();
	pid = fork();
	if (pid == -1)
		return (ResponseUtils::InternalServerError500_NoBody(client.Response), (void)0);
	if (!pid)
	{
		dup2(fd[0], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		execve(args[0], args, env);
		clean(env);
	}
	KqueueUtils::RegisterEvents(kq, fd[1], true);
}

void	Cgi::run(char **env, int kq)
{
	setEnv(env);
	fillExecArgs();
	execFile(kq);
}