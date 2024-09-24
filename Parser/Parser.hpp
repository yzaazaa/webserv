/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/17 02:45:03 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/21 09:21:14 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <map>
# include <iostream>
# include "../ServerInstance/ServerInstance.hpp"
#include "Validator.hpp"
		
class Parser
{
	private:
		Parser();
		Parser(const Parser& other);
		Parser& operator=(const Parser& other);
	
		std::map<int, ServerInstance>	_parsedServers;

	public:
		// Constructors
		Parser(int argc, char **argv);
	
		// Functions
		std::map<int, ServerInstance>&	GetParsedServers();

		// Destructor
		~Parser();
};
