/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorDispatcher.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 06:49:38 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/20 08:20:19 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include "Tokenizer.hpp"
# include "Validator.hpp"

struct DirectiveUnit;
struct TokenSequence;
typedef std::map<std::string, DirectiveUnit>	DirectiveMap;

/// @brief Class handles formating and throwing exceptions for some parts of Validation
class ErrorDispatcher
{
	private:
		ErrorDispatcher();
		ErrorDispatcher(const ErrorDispatcher& other);
		ErrorDispatcher& operator=(const ErrorDispatcher& other);

	public:
		// Functions
		/// @brief Validates the argument count for the directive, ensuring it falls within the accepted range.
		static void	ValidateArgCount(const DirectiveUnit& conf, TokenSequence& seq);
		/// @brief Verifies if the directive is allowed at the current context level.
		static void	ValidateContextLevel(const DirectiveUnit& conf, TokenSequence& seq);
		/// @brief Ensures the directive has a valid and correct delimiter.
		static void	ValidateDelimiter(const DirectiveUnit& conf, TokenSequence& seq);
		/// @brief Validates the directive, ensuring it is supported, and handles any delimiters without an attached directive.
		static bool	ValidateUnknownDirective(DirectiveMap& map, TokenSequence& seq);
		/// @brief Checks if the end of the file is valid, ensuring no hanging or unclosed tokens are present.
		static void	ValidateEndOfFile(TokenSequence& seq, int finalContextlevel, int finalLine);
		/// @brief Checks if the directive has reached or exceeded the maximum allowed duplication count.
		static void	ValidateDuplicateCount(DirectiveUnit& conf, TokenSequence& seq);
		/// @brief Throws an exception for an unknown or unsupported directive found in the token sequence.
		static void	ThrowUnknownDirectiveException(TokenSequence& seq);
		/// @brief Throws an exception for invalid string formatting, ensuring proper quotation and spacing rules are followed.
		static void	ThrowStringFormatError(Token& token, size_t symbLoation);
};
