/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/19 04:36:36 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/20 09:28:04 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include <vector>
# include <fstream>

struct	Token
{
	Token()
	{
		Line = -1;
		Position = -1;
	}
	std::string	Text;
	int			Line;
	int			Position;
};

/// @brief Stores the result of the tokenization process.
struct	TokenizationData
{
	TokenizationData()
	{
		IsString = false;
		IsComment = false;
		SkipNewLine = false;
		QuoteType = ' ';
		LastContextLevel = -1;
	}

	std::vector<Token>			Tokens;
	std::string					LeftOverString;
	std::string					Line;
	bool						IsString;
	bool						IsComment;
	bool						SkipNewLine;
	char						QuoteType;
	int							LastContextLevel;
};

typedef std::vector<Token>::iterator	TokenIter;
extern const char						Delimiters[];
extern const int						DelimitersLen;

/// @brief Class responsable for creating tokens
class Tokenizer
{
	private:
		Tokenizer();
		Tokenizer(const Tokenizer& other);
		Tokenizer& operator=(const Tokenizer& other);

		// Private Functions
		static void		ConfigIsComment(TokenizationData& tokenData, int i);
		static void		ConfigLeftOverString(TokenizationData& tokenData);
		static void		ConfigIsString(TokenizationData& tokenData, int i);
		static Token	ExtractNewToken(TokenizationData& tokenData, int i, int j);
		static void		ConfigureAndPushToken(TokenizationData& tokenData, Token token, int lineCount, int position);
		static bool		DidTokenEnd(TokenizationData& tokenData, size_t i, size_t j);
		static void		TokenizeLine(TokenizationData& tokenData, int lineCount);

	public:
		// Functions
		static void	TokenizeFile(TokenizationData& tokenData, const std::string& argv);
		static bool	IsDelimiter(char c);
		static bool	IsTokenAComment(const Token& token);
};
