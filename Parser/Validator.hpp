/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Validator.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/19 05:00:57 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/20 09:30:25 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include "Tokenizer.hpp"
# include "Parser.hpp"
# include "ErrorDispatcher.hpp"
# include <cstddef>
# include <sstream>
# include <vector>
# include "DirectiveMapper.hpp"

/// @brief Represents an arbitrarily large value used to indicate infinite entries in the directive config.
#define INFINIT_CONF_COUNT 5000

/// @brief Represents a context, consisting of a sequence with one directive,
/// followed by zero or more arguments, and ending with a delimiter.
struct	TokenSequence
{
	TokenSequence() : CtxLevel(-1)
	{	}
	
	void	Reset()
	{
		Token token;
		Directive = token;
		Arguments.clear();
		Delimiter = token;
	}

	bool	IsDefault()
	{
		return (Directive.Text.empty() && Arguments.empty() && Delimiter.Text.empty());
	}

	int					CtxLevel;
	Token				ContextDirective;
	Token				Directive;
	std::vector<Token>	Arguments;
	Token				Delimiter;
};

/// @brief Configuration settings for a specific directive.
struct DirectiveUnit
{
    int		MinCtxLvl;
    int		MaxCtxLvl;
	int		MaxDup;
	std::string	RequiredDelimiter;
	size_t		MinArg;
	size_t		MaxArg;
	
	DirectiveUnit() :
			MinCtxLvl(-1), MaxCtxLvl(-1), MaxDup(-1),
			RequiredDelimiter("-1"), MinArg(0), MaxArg(0)
	{
		
	}
	DirectiveUnit(int mnCtx, int mxCtx, int mxDup, const std::string& reqDel, size_t mnArg, size_t mxArg) :
		MinCtxLvl(mnCtx), MaxCtxLvl(mxCtx), MaxDup(mxDup), RequiredDelimiter(reqDel), MinArg(mnArg), MaxArg(mxArg)
	{
		
	}
};

/// @brief Maps directive names to their corresponding configuration data.
typedef std::map<std::string, DirectiveUnit>				DirectiveMap;
/// @brief Maps directive names to their respective duplication counts.
typedef std::map<std::string, size_t>					CtxDirectoryCountMap;
/// @brief Maps context levels to a collection of directive names and their duplication counts.
typedef std::map<size_t, CtxDirectoryCountMap>				DuplicationMap;
/// @brief Iterator type for traversing the DuplicationMap.
typedef std::map<size_t, CtxDirectoryCountMap>::iterator	DuplicationMapIter;

extern const char*	Directives[];
extern const int	DirectivesLen;


class Validator
{
	private:
		// Private Constructors
		Validator(const Validator& other);
		Validator();

		// Private Operators
		Validator& operator=(const Validator& other);

		// Private Functions
		/// @brief Remove a tokens string quotes
		static void	CleanTokenString(Token& token, TokenSequence& seq, bool isDirective);
		/// @brief Remove quotes and handle errors for an entire TokenSequence
		static void	CleanSequenceStrings(TokenSequence& seq);
		/// @brief Validate a single TokenSequence
		static void	ValidateSeq(TokenSequence& seq, DirectiveMap& map);

		static void	PrepareSeqToken(TokenSequence& seqToken, Token& token, int ctxLvl);
		static void	UpdateContextDirective(TokenSequence& seqToken, std::map<int, TokenSequence>& ctxMap);

	public:
		// Functions
		static void	Validate(TokenizationData& tokenizationData, std::vector<TokenSequence>& tokenSeqances);
};
