/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInitializer.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/17 02:41:15 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/23 14:53:28 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include "../ServerInstance/ServerInstance.hpp"
# include "../Parser/Parser.hpp"
# include "../Server/Server.hpp"
# include <netdb.h>
# include <sys/types.h>
# include <sys/socket.h>

class ServerInitializer
{
	private:
		ServerInitializer();
		ServerInitializer(const ServerInitializer& other);
		
		bool	_isAnySocketOpen;
		
		// Private functions
		void	InitializeServer(Server& server, Parser& parser);
		void	ConnectServers(Server& server, ServerInstance& instance, const ListenInfo& listenInfo);
		

	public:
		// Constructors
		ServerInitializer(Server& server, Parser& parser);

		// Operators
		ServerInitializer& operator=(const ServerInitializer& other);

		// Destructor
		~ServerInitializer();
};
