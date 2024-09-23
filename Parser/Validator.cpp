/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validator.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/19 05:01:02 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/20 09:30:20 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Validator.hpp"

/// *** Constructors *** ///
#pragma region Constructors

Validator::Validator()
{

}

Validator::Validator(const Validator& other)
{
	*this = other;
}

#pragma endregion


/// *** Functions *** ///
#pragma region Functions

void	Validator::PrepareSeqToken(TokenSequence& seqToken, Token& token, int ctxLvl)
{
	if (seqToken.IsDefault())
	{
		seqToken.CtxLevel = ctxLvl;
		seqToken.Directive.Line = token.Line;
	}
	seqToken.Delimiter = token;
}

void	Validator::UpdateContextDirective(TokenSequence& seqToken, std::map<int, TokenSequence>& ctxMap)
{
	if (seqToken.CtxLevel != 1)
		seqToken.ContextDirective = ctxMap[seqToken.CtxLevel - 1].Directive;
	ctxMap[seqToken.CtxLevel] = seqToken;
}

void	Validator::CleanTokenString(Token& token, TokenSequence& seq, bool isDirective)
{
	char	symbol;
	size_t	symbolLocation;

	if (token.Text.empty() || token.Text.size() <= 1)
		return;
	if (token.Text[0] != '\'' && token.Text[0] != '\"')
		return;
	
	symbol = token.Text[0];
	
	symbolLocation = 1;
	while (symbolLocation < token.Text.size() && token.Text[symbolLocation] != symbol)
		symbolLocation++;
	
	// string is not valid
	if (!(symbolLocation == token.Text.size() - 1))
		ErrorDispatcher::ThrowStringFormatError(token, symbolLocation);
	
	// Stirng is valid
	token.Text = token.Text.substr(1, token.Text.size() - 2);
	if (token.Text.empty() && isDirective)
		ErrorDispatcher::ThrowUnknownDirectiveException(seq);
}

void	Validator::CleanSequenceStrings(TokenSequence& seq)
{
	CleanTokenString(seq.Directive, seq, true);
	for (TokenIter it =  seq.Arguments.begin(); it != seq.Arguments.end(); ++it)
		CleanTokenString(*it, seq, false);
}

void	Validator::ValidateSeq(TokenSequence& seq, DirectiveMap& map)
{
	CleanSequenceStrings(seq);
	if (!ErrorDispatcher::ValidateUnknownDirective(map, seq))
		return;
	DirectiveUnit& conf = map[seq.Directive.Text];
	ErrorDispatcher::ValidateContextLevel(conf, seq);
	ErrorDispatcher::ValidateArgCount(conf, seq);
	ErrorDispatcher::ValidateDelimiter(conf, seq);
	ErrorDispatcher::ValidateDuplicateCount(conf, seq);
}

void	Validator::Validate(TokenizationData& tokenizationData, std::vector<TokenSequence>& tokenSeqances)
{
	TokenSequence	seqToken;
	int				contextlevel;
	int				lastLine;
	std::map<int, TokenSequence> contextMap;
	DirectiveMap				directiveMap;

	seqToken.Reset();
	contextlevel = 1;
	lastLine = 0;
	DirectiveMapper::BuildDirectiveMap(directiveMap);
	// Build TokenSequences
	for (TokenIter it =  tokenizationData.Tokens.begin(); it != tokenizationData.Tokens.end(); ++it)
	{
		Token& token = *it;
		if (token.Line != -1)
			lastLine = token.Line;
		if (Tokenizer::IsTokenAComment(token))
			continue;
			
		if (Tokenizer::IsDelimiter(token.Text[0]))
		{
			if (token.Text[0] == '{')
				contextlevel++;
			if (token.Text[0] == '}')
				contextlevel--;
			PrepareSeqToken(seqToken, token, contextlevel);
			UpdateContextDirective(seqToken, contextMap);
			ValidateSeq(seqToken, directiveMap);
			tokenSeqances.push_back(seqToken);
			seqToken.Reset();
			continue;
		}
		// if the tokenSeq has no data the first data is a Directive
		if (seqToken.IsDefault())
		{
			seqToken.Directive = token;
			seqToken.CtxLevel = contextlevel;
			continue;
		}
		// if not Default and no delimiter then its an argument
		seqToken.Arguments.push_back(token);
	}
	ErrorDispatcher::ValidateEndOfFile(seqToken, contextlevel, lastLine);
}

#pragma endregion


/// *** Operators *** ///
#pragma region Operators

Validator& Validator::operator=(const Validator& other)
{
	if (this != &other)
	{
		
	}
	return *this;
}

#pragma endregion
