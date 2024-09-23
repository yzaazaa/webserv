/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/19 04:39:54 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/20 07:08:04 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Tokenizer.hpp"

const char	Delimiters[] = { '{', '}', ';'};
const int	DelimitersLen = sizeof(Delimiters) / sizeof(Delimiters[0]);

/// *** Constructors *** ///
#pragma region Constructors

Tokenizer::Tokenizer()
{

}

Tokenizer::Tokenizer(const Tokenizer& other)
{
	*this = other;
}

#pragma endregion

/// *** Functions *** ///
#pragma region Functions

bool	Tokenizer::IsDelimiter(char c)
{
	for (int i = 0; i < DelimitersLen; i++)
	{
		if (c == Delimiters[i])
			return (true);
	}
	return (false);
}

bool	Tokenizer::IsTokenAComment(const Token& token)
{
	if (!token.Text.empty() && token.Text[0] == '#')
		return (true);
	return (false);
}

void	Tokenizer::ConfigIsComment(TokenizationData& tokenData, int i)
{
	tokenData.IsComment = !tokenData.IsString && tokenData.Line[i] == '#';
}

void	Tokenizer::ConfigLeftOverString(TokenizationData& tokenData)
{
	tokenData.LeftOverString = "";
	if (tokenData.IsString && !tokenData.Tokens.empty())
	{
		tokenData.LeftOverString = tokenData.Tokens.back().Text;
		if (!tokenData.SkipNewLine)
			tokenData.LeftOverString += '\n';
		tokenData.Tokens.pop_back();
	}
}

void	Tokenizer::ConfigIsString(TokenizationData& tokenData, int i)
{
	bool	isFirstCharacter = (i <= 0);
	bool	isPreveusCharacterSpace = (i > 0 && std::isspace(tokenData.Line[i - 1]));
	bool	isQuotationMark = (tokenData.Line[i] == '\'' || tokenData.Line[i] == '\"');
	
	if (tokenData.IsComment)
		return;
	if (tokenData.IsString || !isQuotationMark)
	{
		if (isQuotationMark && (tokenData.QuoteType == tokenData.Line[i]))
			tokenData.IsString = false;
		return;
	}
	if (!isFirstCharacter && !isPreveusCharacterSpace)
		return;
	tokenData.QuoteType = tokenData.Line[i];
	tokenData.IsString = true;
}

bool	Tokenizer::DidTokenEnd(TokenizationData& tokenData, size_t i, size_t j)
{
	bool	isLastElement = (j == tokenData.Line.size() - 1);

	if (tokenData.Line[j] == tokenData.QuoteType && i != j)
		tokenData.IsString = false;
	
	if (tokenData.IsString && !isLastElement)
		return (false);
	else if (!isLastElement)
	{
		if (tokenData.IsComment)
			return (false);
		if (!std::isspace(tokenData.Line[j]) &&
				!IsDelimiter(tokenData.Line[j]) &&
				!IsDelimiter(tokenData.Line[j + 1]))
			return (false);
	}
	return  (true);
}

Token	Tokenizer::ExtractNewToken(TokenizationData& tokenData, int i, int j)
{
	size_t	startPosition = i;
	size_t	endPosition = j;
	size_t	length;
	Token	token;

	if (std::isspace(tokenData.Line[j]) && !tokenData.IsString)
		endPosition--;
	length = endPosition - startPosition + 1;
	tokenData.LeftOverString += tokenData.Line.substr(startPosition, length);

	token.Text = tokenData.LeftOverString;
	return (token);
}

void	Tokenizer::ConfigureAndPushToken(TokenizationData& tokenData, Token token, int lineCount, int position)
{
	token.Line = lineCount;
	token.Position = position;
	tokenData.Tokens.push_back(token);
}

void	Tokenizer::TokenizeLine(TokenizationData& tokenData, int lineCount)
{
	tokenData.SkipNewLine = (0 >= tokenData.Line.size() && tokenData.IsString);
	if (0 >= tokenData.Line.size() && tokenData.IsString)
		tokenData.Line += "\n";
	for (size_t i = 0; i < tokenData.Line.size(); i++)
	{
		if (std::isspace(tokenData.Line[i]) && !tokenData.IsString)
			continue;
		ConfigIsComment(tokenData, i);
		ConfigLeftOverString(tokenData);
		ConfigIsString(tokenData, i);
		
		for (size_t j = i; j < tokenData.Line.size(); j++)
		{
			if (!DidTokenEnd(tokenData, i, j))
				continue;
			ConfigureAndPushToken(tokenData, ExtractNewToken(tokenData, i, j), lineCount, i + 1);
			i = j;
			break;
		}
	}
}

void	Tokenizer::TokenizeFile(TokenizationData& tokenData, const std::string& argv)
{
	std::ifstream	file(argv);
	int				lineCount;

	if (!file.is_open())
		throw std::runtime_error("Could not open file.");

	lineCount = 1;
	while (std::getline(file,  tokenData.Line))
		TokenizeLine(tokenData, lineCount++);
}

#pragma endregion

/// *** Operators *** ///
#pragma region Operators

Tokenizer& Tokenizer::operator=(const Tokenizer& other)
{
	if (this != &other)
	{
		
	}
	return *this;
}

#pragma endregion
