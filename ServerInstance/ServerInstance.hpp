/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInstance.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 03:38:06 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/15 04:57:26 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <cstddef>
#include <string>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h> 
# include <set>
# include <map>

struct	ClientMaxBodySize
{
	ClientMaxBodySize() : IsDefined(false), Bytes(0), Kilobytes(0), Megabyte(1), Gigabyte(0)
	{	}
	ClientMaxBodySize(size_t bytes, size_t kilobytes, size_t megabyte, size_t gigabyte) :
			IsDefined(true), Bytes(bytes), Kilobytes(kilobytes), Megabyte(megabyte), Gigabyte(gigabyte)
	{	}
	ClientMaxBodySize(const ClientMaxBodySize& other) :
			 IsDefined(false), Bytes(other.Bytes), Kilobytes(other.Kilobytes), Megabyte(other.Megabyte), Gigabyte(other.Gigabyte)
	{	}

	bool	IsDefined;
	size_t	Bytes;
	size_t	Kilobytes;
	size_t	Megabyte;
	size_t	Gigabyte;
};

struct	ErrorPageData
{
	ErrorPageData() : IsDefined(false)
	{	}
	ErrorPageData(const ErrorPageData& other) : IsDefined(false), ErrorPages(other.ErrorPages)
	{	}

	void	AddEntry(int code, const std::string& value)
	{
		IsDefined = true;
		ErrorPages[code] = value;
	}
	bool	IsDefined;
	std::map<int, std::string> ErrorPages;
};

struct	AutoIndexData
{
	AutoIndexData() : IsDefined(false)
	{	}
	AutoIndexData(const AutoIndexData& other) : IsDefined(false), IsAutoIndexOn(other.IsAutoIndexOn)
	{	}
	AutoIndexData(bool isAutoIndexOn) : IsDefined(true), IsAutoIndexOn(isAutoIndexOn)
	{	}

	bool	IsDefined;
	bool	IsAutoIndexOn;
};

struct	IndexData
{
	IndexData() : IsDefined(false)
	{	}
	IndexData(const IndexData& other) : IsDefined(false), Index(other.Index)
	{	}

	bool					IsDefined;
	std::set<std::string>	Index;
};

struct ListenInfo
{
	ListenInfo() : Address("0.0.0.0"), Port("-1")
	{	}
	ListenInfo(const std::string& address, const std::string& port) :
				Address(address.empty() ? "0.0.0.0" : address), Port(port)
	{	}

	std::string	Address;
	std::string	Port;

	std::string ToString() const
	{
		if (Address.empty())
			return  Port;
		return Address + " " + Port;
	}

	bool operator==(const ListenInfo& other) const
	{
        return (Address == other.Address && Port == other.Port);
    }

    bool operator<(const ListenInfo& other) const
	{
        if (Address < other.Address) return (true);
        if (Address > other.Address) return (false);
        return (Port < other.Port);
    }
};

struct RedirectionData
{
	RedirectionData() : IsDefined(false), Code(-1)
	{	}
	RedirectionData(int code, const std::string& value) : IsDefined(true), Code(code), Value(value)
	{	}

	bool	IsDefined;
	int		Code;
	std::string	Value;
};

struct RootDirectoryData
{
	RootDirectoryData() : IsOverriten(false), IsDefined(false)
	{	}
	RootDirectoryData(const RootDirectoryData& other) : IsOverriten(false), IsDefined(other.IsDefined), Value(other.Value)
	{	}
	RootDirectoryData(const std::string& value) : IsOverriten(true), IsDefined(true), Value(value)
	{	}

	bool	IsOverriten;
	bool	IsDefined;
	std::string	Value;
};

struct CgiPassData
{
	CgiPassData() : IsDefined(false)
	{	}
	CgiPassData(const CgiPassData& other) : IsDefined(false), Value(other.Value)
	{	}

	bool	IsDefined;
	std::map<std::string, std::string>	Value;
};

struct HttpInstance
{
	ClientMaxBodySize			MaxBodySize;
	ErrorPageData				ErrorPages;
	AutoIndexData 				AutoIndex;
	RootDirectoryData			Root;
	IndexData 					Index;
	CgiPassData 				CgiPass;
};

struct LocationInstance
{
	LocationInstance()
	{	}
	LocationInstance(const std::string& arg) : PathArg(arg)
	{	}
	std::string			PathArg;
	ClientMaxBodySize	ClientMaxBody;
	ErrorPageData		ErrorPages;
	RootDirectoryData		RootDirectory;
	AutoIndexData		IsAutoIndex;
	IndexData			IndexValue;
	CgiPassData			CgiConfig;

	std::set<std::string>	AllowedMethods;
	RedirectionData			Redirection;
	
};


struct ServerInstance 
{
	ServerInstance() : ServerIndex(-1)
	{	}
	ServerInstance(int serverIndex) : ServerIndex(serverIndex)
	{	}

	/// @brief Server Position in the config file
	int ServerIndex;
	std::set<ListenInfo>	ServerSockets;
	std::set<std::string>	ServerNames;
	ClientMaxBodySize		ClientMaxBody;
	ErrorPageData			ErrorPages;
	RootDirectoryData		RootDirectory;
	AutoIndexData			IsAutoIndex;
	IndexData  				IndexValue;
	CgiPassData				CgiConfig;

	std::map<std::string, LocationInstance> Locations;

	// Operators
	bool operator==(const ServerInstance& other) const
	{
		if (this == &other)
				return (true);
		return (ServerIndex == other.ServerIndex);
	}
};




typedef	std::map<std::string, LocationInstance>::iterator LocationsIter;

typedef	std::map<int, ServerInstance>::iterator ServerInstanceIterator;
typedef std::set<ListenInfo>::iterator ListenInfoIterator;