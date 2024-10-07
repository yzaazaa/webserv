#include "Cgi.hpp"
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <utility>
#include "../Http/Client.hpp"

Cgi::Cgi(Client &client) : client(client), ready(false) {}

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

void	Cgi::clean()
{
	if (pid != 0)
		kill(pid, SIGKILL);
	dup2(stdFd[0], STDIN_FILENO);
	dup2(stdFd[1], STDOUT_FILENO);
	close(fd[0]);
	close(fd[1]);
	close(stdFd[0]);
	close(stdFd[1]);
	if (_env)
	{
		for (int i = 0; _env[i] != NULL; i++)
			delete[] _env[i];
		delete[] _env;
	}
}

void	Cgi::execFile(int kq, Server &server)
{
	try
	{
		_env = getEnvStrs();
	}
	catch (std::exception &e)
	{
		clean();
		ResponseUtils::InternalServerError500_NoBody(client.Response);
		throw ;
	}
	pid = fork();
	if (pid == -1)
		return (ResponseUtils::InternalServerError500_NoBody(client.Response), (void)0);
	if (!pid)
	{
		if (dup2(fd[0], STDIN_FILENO) == -1)
		{
			clean();
			throw std::runtime_error("Execve fail!");
		}
		if (dup2(fd[1], STDOUT_FILENO) == -1)
		{
			clean();
			throw std::runtime_error("Execve fail!");
		}
		execve(args[0], args, _env);
		clean();
		throw std::runtime_error("Execve fail!");
	}
	KqueueUtils::RegisterEvents(kq, fd[1], true);
	server.addFd(fd[1], client.socket);
}

void	Cgi::prepareResponse(Server& server, int kq, int fd)
{
	ready = true;
	KqueueUtils::DeleteEvents(kq, fd);
	clean();
	server.eraseFd(fd);
	KqueueUtils::EnableEvent(kq, client.socket, WRITE);
	client.Response.IsLastResponse = true;
	client.Response.CloseConnection = true;
}

void	Cgi::prepareFds(Server& server, int kq)
{
	if (pipe(fd) == -1)
		return (ResponseUtils::InternalServerError500_NoBody(client.Response), (void)0);
	server.addFd(fd[0], client.socket);
	KqueueUtils::RegisterEvents(kq, fd[0]);
	KqueueUtils::DisableEvent(kq, fd[0], READ);
	KqueueUtils::EnableEvent(kq, fd[0], WRITE);
	if ((stdFd[0] = dup(STDIN_FILENO)) == -1)
		return (ResponseUtils::InternalServerError500_NoBody(client.Response), (void)0);
	if ((stdFd[1] = dup(STDOUT_FILENO)) == -1)
		return (ResponseUtils::InternalServerError500_NoBody(client.Response), (void)0);
}

void	Cgi::run(char **env, int kq, Server &server)
{
	setEnv(env);
	fillExecArgs();
	prepareFds(server, kq);
}

bool	Cgi::getStatus()
{
	return ready;
}