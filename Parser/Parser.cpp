/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/17 02:46:31 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/23 14:57:22 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Parser.hpp"


/// *** Constructors *** ///
#pragma region Constructors

Parser::Parser()
{	}

Parser::Parser(const Parser& other)
{
	*this = other;
}



Parser::Parser(int argc, char **argv)
{
	if (argc <= 1)
		throw std::runtime_error("Configuration file path is required.");
	else if (argc > 2)
		throw std::runtime_error("Multiple configuration files are not allowed.");

	TokenizationData tokenizationData;
	Tokenizer::TokenizeFile(tokenizationData, argv[1]);

	std::vector<TokenSequence> tokenSeuqnces;
	Validator::Validate(tokenizationData, tokenSeuqnces);
	
	DirectiveMapper::StartConfigMapping(*this, tokenSeuqnces);
}

#pragma endregion

/// *** Functions *** ///
#pragma region Functions

std::map<int, ServerInstance>&	Parser::GetParsedServers()
{
	return (_parsedServers);
}

#pragma endregion

/// *** Operators *** ///
#pragma region Operators

Parser& Parser::operator=(const Parser& other)
{
	if (this != &other)
	{
		
	}
	return *this;
}

#pragma endregion

/// *** Destructor *** ///
#pragma region Destructor

Parser::~Parser()
{

}

#pragma endregion