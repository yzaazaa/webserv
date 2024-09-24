/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Debug.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/17 03:37:18 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/23 18:51:47 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Debug.hpp"
# include <vector>

void PrintBodySize(ClientMaxBodySize& bodySize)
{
	std::cout << "Bytes: " << bodySize.Bytes;
	std::cout << " Kilobytes: " << bodySize.Kilobytes;
	std::cout << " Megabyte: " << bodySize.Megabyte;
	std::cout << " Gigabyte: " << bodySize.Gigabyte;
}

void PrintParsedServer(Parser& parser)
{
	std::cout << std::endl << "\b\t Parsed Server Instances" << std::endl;

	std::map<int, ServerInstance>& servers = parser.GetParsedServers();
	for (ServerInstanceIterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt)
    {
		ServerInstance& instance = serverIt->second;
		std::cout << "ServerIndex: " << instance.ServerIndex << std::endl;
		std::cout << "\tServerSockets: ";
		for (ListenInfoIterator listenIt = instance.ServerSockets.begin(); listenIt != instance.ServerSockets.end(); ++listenIt)
		{
			std::cout << listenIt->ToString();
			if ((++listenIt) != instance.ServerSockets.end())
				std::cout << ", ";
			listenIt--; 
		}
		
		std::cout << "\n\tServerNames: ";
		for (std::set<std::string>::iterator listenIt = instance.ServerNames.begin(); listenIt != instance.ServerNames.end(); ++listenIt)
		{
			std::cout << *listenIt;
			if ((++listenIt) != instance.ServerNames.end())
				std::cout << ", ";
			listenIt--; 
		}
		std::cout << "\n\tErrorPages: ";
		for (std::map<int, std::string>::iterator listenIt = instance.ErrorPages.ErrorPages.begin(); listenIt != instance.ErrorPages.ErrorPages.end(); ++listenIt)
		{
			std::cout << "[" << listenIt->first << "][" << listenIt->second << "]";
			if ((++listenIt) != instance.ErrorPages.ErrorPages.end())
				std::cout << ", ";
			listenIt--; 
		}
		std::cout << "\n\tCgi Pass: ";
		for (std::map<std::string, std::string>::iterator listenIt = instance.CgiConfig.Value.begin(); listenIt != instance.CgiConfig.Value.end(); ++listenIt)
		{
			std::cout << "[" << listenIt->first << "][" << listenIt->second << "]";
			if ((++listenIt) != instance.CgiConfig.Value.end())
				std::cout << ", ";
			listenIt--; 
		}
		std::cout << "\n\tServer Index: ";
		for (std::set<std::string>::iterator listenIt = instance.IndexValue.Index.begin(); listenIt != instance.IndexValue.Index.end(); ++listenIt)
		{
			std::cout << *listenIt;
			if ((++listenIt) != instance.IndexValue.Index.end())
				std::cout << ", ";
			listenIt--; 
		}
		
		std::cout << "\n\tServer_max_body_size: ";
		PrintBodySize(instance.ClientMaxBody);
		std::cout << std::endl;
		
		std::cout << "\n\tAutoIndex: ";
		std::cout << (instance.IsAutoIndex.IsAutoIndexOn ? "True" : "False") << std::endl;
		
		if (instance.RootDirectory.IsDefined)
		{
			std::cout << "\n\tServer Root: ";
			std::cout << instance.RootDirectory.Value << std::endl;
		}
		
		for (std::map<std::string, LocationInstance>::iterator listenIt = instance.Locations.begin(); listenIt != instance.Locations.end(); ++listenIt)
		{
			LocationInstance& location = listenIt->second;
			std::cout << "\n\tLocation: ";
			std::cout << listenIt->second.PathArg;
			std::cout << "\n\t\tLocation_max_body_size: ";
			PrintBodySize(listenIt->second.ClientMaxBody);
			
			if (location.Redirection.IsDefined)
			{
				std::cout << "\n\t\tRedirects: ";
				std::cout << location.Redirection.Code << ", " << location.Redirection.Value;
			}
			if (location.RootDirectory.IsDefined)
			{
				std::cout << "\n\t\tRoot: ";
				std::cout << location.RootDirectory.Value;
			}
			
			std::cout << "\n\t\tAutoIndex: ";
			std::cout << (location.IsAutoIndex.IsAutoIndexOn ? "True" : "False") << std::endl;
		
			std::cout << "\n\t\tErrorPages: ";
			for (std::map<int, std::string>::iterator listenIt = location.ErrorPages.ErrorPages.begin(); listenIt != location.ErrorPages.ErrorPages.end(); ++listenIt)
			{
				std::cout << "[" << listenIt->first << "][" << listenIt->second << "]";
				if ((++listenIt) != location.ErrorPages.ErrorPages.end())
					std::cout << ", ";
				listenIt--; 
			}
			std::cout << "\n\t\tCgi Pass: ";
			for (std::map<std::string, std::string>::iterator listenIt = location.CgiConfig.Value.begin(); listenIt != location.CgiConfig.Value.end(); ++listenIt)
			{
				std::cout << "[" << listenIt->first << "][" << listenIt->second << "]";
				if ((++listenIt) != location.CgiConfig.Value.end())
					std::cout << ", ";
				listenIt--; 
			}
			std::cout << "\n\t\tAllowed_Methods: ";
			for (std::set<std::string>::iterator listenIt = location.AllowedMethods.begin(); listenIt != location.AllowedMethods.end(); ++listenIt)
			{
				std::cout << *listenIt;
				if ((++listenIt) != location.AllowedMethods.end())
					std::cout << ", ";
				listenIt--; 
			}
			std::cout << "\n\t\tIndex: ";
			for (std::set<std::string>::iterator listenIt = location.IndexValue.Index.begin(); listenIt != location.IndexValue.Index.end(); ++listenIt)
			{
				std::cout << *listenIt;
				if ((++listenIt) != location.IndexValue.Index.end())
					std::cout << ", ";
				listenIt--; 
			}
		}
		std::cout << std::endl;
	}
}