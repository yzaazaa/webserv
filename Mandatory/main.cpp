/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:06:39 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/23 18:51:19 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../Parser/Parser.hpp"
# include "../Server/Server.hpp"
# include "../Server/ServerInitializer.hpp"
# include <cstdlib>
# include "../Debug/Debug.hpp"

int	main(int argc, char **argv)
{
	try
	{
		Parser parser(argc, argv);
		Server server;
		//PrintParsedServer(parser);
		ServerInitializer serverInitializer(server, parser);

		while (1)
		{

		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}