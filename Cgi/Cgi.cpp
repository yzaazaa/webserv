#include "Cgi.hpp"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <strstream>
#include <sys/event.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <fstream>
#include "../Http/Client.hpp"

Cgi::Cgi() :  _env(NULL), finished(0)
{

}

Cgi::~Cgi() {}

void	Cgi::setClient(Client *_client)
{
	client = _client;
}

void	Cgi::getEnvMap(char **env)
{
	for (int i = 0; env[i] != NULL; i++)
	{
		char	*equal = std::strchr(env[i], '=');
		if (equal)
		{
			size_t	lenkey = equal - env[i];
			std::string key(env[i], lenkey);
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
	std::cout << client->Request.uri.path << "\n";
	envMap.insert(std::make_pair("SERVER_NAME", client->Request.headers["host"].substr(0, client->Request.headers["host"].find(':'))));
	envMap.insert(std::make_pair("GATEWAY_INTERFACE", "CGI/1.1"));
	envMap.insert(std::make_pair("SERVER_PROTOCOL", "HTTP/1.1"));
	envMap.insert(std::make_pair("SERVER_PORT", "8090"));
	envMap.insert(std::make_pair("REMOTE_ADDR", "0.0.0.0"));
	envMap.insert(std::make_pair("REQUEST_METHOD", client->Request.method));
	envMap.insert(std::make_pair("SCRIPT_NAME", client->Request.uri.path));
	envMap.insert(std::make_pair("QUERY_STRING", client->Request.uri.query));
	envMap.insert(std::make_pair("CONTENT_TYPE", client->Request.headers["content-type"].substr(0, client->Request.headers["content-type"].find(';'))));
	envMap.insert(std::make_pair("CONTENT_LENGTH", std::to_string(client->Request.body.length())));
}

void	Cgi::fillExecArgs()
{
	if (client->Request.uri.extension == ".py")
		scriptExecutor = PYTHON;
	else if (client->Request.uri.extension == ".java")
		scriptExecutor = JAVA;
	else
		scriptExecutor = PHP;
	args[0] = const_cast<char *>(scriptExecutor.c_str());
	args[1] = const_cast<char *>(client->Request.uri.path.c_str());
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

void	Cgi::clean(Server& server, int kq)
{
	KqueueUtils::DeleteEvents(kq, fd[0], 2);
	server.eraseFd(fd[0]);
	server.eraseFd(fd[2]);
	close(fd[0]);
	close(fd[1]);
	close(fd[2]);
	close(stdFd[0]);
	close(stdFd[1]);
	close(stdFd[2]);
	if (_env)
	{
		for (int i = 0; _env[i] != NULL; i++)
			delete[] _env[i];
		delete[] _env;
	}
}

int		Cgi::wait(Server& server, int kq, int flag)
{
	int status;
	int res = waitpid(pid, &status, WNOHANG);
	if (res == pid)
	{
		if (!WEXITSTATUS(status) || WEXITSTATUS(status) == 2 || WEXITSTATUS(status) == 3 || WEXITSTATUS(status) == 4)
			dup2(stdFd[0], STDIN_FILENO);
		if (!WEXITSTATUS(status) || WEXITSTATUS(status) == 3 || WEXITSTATUS(status) == 4)
			dup2(stdFd[1], STDOUT_FILENO);
		if (!WEXITSTATUS(status) || WEXITSTATUS(status) == 4)
			dup2(stdFd[2], STDOUT_FILENO);
		std::cout << "Finished executing\n";
		KqueueUtils::DeleteEvents(kq, fd[0], 2);
		close(fd[0]);
		server.eraseFd(fd[0]);
		close(fd[2]);
		server.eraseFd(fd[2]);
		close(stdFd[0]);
		close(stdFd[1]);
		close(stdFd[2]);
		if (_env)
		{
			for (int i = 0; _env[i] != NULL; i++)
				delete[] _env[i];
			delete[] _env;
		}
		std::cout << "status: " << WEXITSTATUS(status) << std::endl;
		if (WEXITSTATUS(status) || lseek(fd[1], 0, SEEK_SET) == -1)
		{
			if (flag)
			{
				KqueueUtils::DeleteEvents(kq, fd[1]);
				server.eraseFd(fd[1]);
			}
			close(fd[1]);
			ResponseUtils::InternalServerError500_NoBody(client->Response, kq, client->socket);
			KqueueUtils::EnableEvent(kq, client->socket, WRITE);
			return (1);
		}
		std::cout << "Cgi script finished normally\n";
		finished = true;
		return (0);
	}
	if (flag && ft_time() - client->lastTime >= TIMEOUT)
	{
		std::cout << "killing process\n";
		kill(pid, SIGKILL);
		KqueueUtils::DeleteEvents(kq, fd[1], 0);
		server.eraseFd(fd[1]);
		client->Cgi.clean(server, kq);
		finished = true;
		ResponseUtils::GatewayTimeout504_NoBody(client->Response, kq, client->socket);
		KqueueUtils::EnableEvent(kq, client->socket, WRITE);
	}
	return (2);
}

void	Cgi::execFile(int kq, Server &server)
{
	try
	{
		_env = getEnvStrs();
	}
	catch (std::exception &e)
	{
		clean(server, kq);
		ResponseUtils::InternalServerError500_NoBody(client->Response, kq, client->socket);
		throw ;
	}
	client->lastTime = ft_time();
	pid = fork();
	if (pid == -1)
	{
		clean(server, kq);
		return (ResponseUtils::InternalServerError500_NoBody(client->Response, kq, client->socket), (void)0);
	}
	if (!pid)
	{
		if (dup2(fd[0], STDIN_FILENO) == -1)
			exit(1);
		std::cout << "dupped stdin successfully\n";
		if (dup2(fd[1], STDOUT_FILENO) == -1)
			exit(2);
		if (dup2(fd[2], STDERR_FILENO) == -1)
			exit(3);
		execve(args[0], args, _env);
		perror("execve failed\n");
		exit(4);
	}
	int	res = wait(server, kq, 0);
	if (res == 1)
		return ;
	KqueueUtils::RegisterEvents(kq, fd[1]);
	server.addFd(fd[1], client->socket);
	std::cout << "added fd[1] to kqueue\n";
}

void	Cgi::prepareResponse(Server& server, int kq, int fildes)
{
	KqueueUtils::DeleteEvents(kq, fildes);
	server.eraseFd(fildes);
	close(fd[1]);
	KqueueUtils::EnableEvent(kq, client->socket, WRITE);
	client->Response.IsLastResponse = true;
	client->Response.CloseConnection = true;
	client->lastTime = ft_time();
}

void	Cgi::prepareFds(Server& server, int kq)
{
	fd[0] = fileno(tmpfile());
	if (fd[0] == -1)
		std::cout << "Error opening input.cgi\n";
	fd[1] = fileno(tmpfile());
	if (fd[1] == -1)
		std::cout << "Error opening output.cgi\n";
	fd[2] = fileno(tmpfile());
	if (fd[2] == -1)
		std::cout << "Error opening error.cgi\n";
	std::cerr << "fd0: " << fd[0] << "\n";
	std::cerr << "fd1: " << fd[1] << "\n";
	server.addFd(fd[0], client->socket);
	KqueueUtils::RegisterEvents(kq, fd[0]);
	KqueueUtils::DisableEvent(kq, fd[0], READ);
	KqueueUtils::EnableEvent(kq, fd[0], WRITE);
	if ((stdFd[0] = dup(STDIN_FILENO)) == -1)
	{
		KqueueUtils::DeleteEvents(kq, fd[0]);
		server.eraseFd(fd[0]);
		close(fd[0]);
		return (ResponseUtils::InternalServerError500_NoBody(client->Response, kq, client->socket), (void)0);
	}
	if ((stdFd[1] = dup(STDOUT_FILENO)) == -1)
	{
		KqueueUtils::DeleteEvents(kq, fd[0]);
		server.eraseFd(fd[0]);
		close(fd[0]);
		return (ResponseUtils::InternalServerError500_NoBody(client->Response, kq, client->socket), (void)0);
	}
	if ((stdFd[2] = dup(STDERR_FILENO)) == -1)
	{
		KqueueUtils::DeleteEvents(kq, fd[0]);
		KqueueUtils::DeleteEvents(kq, fd[1]);
		server.eraseFd(fd[0]);
		server.eraseFd(fd[1]);
		close(fd[0]);
		close(fd[1]);
		return (ResponseUtils::InternalServerError500_NoBody(client->Response, kq, client->socket), (void)0);
	}
}

void	Cgi::run(char **env, int kq, Server &server)
{
	setEnv(env);
	fillExecArgs();
	prepareFds(server, kq);
	std::cout << "Finished preparing fds\n";
}
