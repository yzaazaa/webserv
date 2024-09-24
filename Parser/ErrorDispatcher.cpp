/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorDispatcher.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 02:44:43 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/23 13:41:45 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ErrorDispatcher.hpp"

/// *** Constructors *** ///
#pragma region Constructors

ErrorDispatcher::ErrorDispatcher()
{

}

ErrorDispatcher::ErrorDispatcher(const ErrorDispatcher& other)
{
	*this = other;
}

#pragma endregion

/// *** Functions *** ///
#pragma region Functions

void	ErrorDispatcher::ThrowUnknownDirectiveException(TokenSequence& seq)
{
	std::ostringstream errorStream;
	errorStream << "Unknown directive: \"" << seq.Directive.Text << "\"";
	errorStream << " (line: " << seq.Directive.Line << ", position: " << seq.Directive.Position << ")";
	throw std::runtime_error(errorStream.str());
}

void	ErrorDispatcher::ThrowStringFormatError(Token& token, size_t symbLoation)
{
	std::ostringstream errorStream;
	
	symbLoation++;
	errorStream << "Unexpected character: \"";
	if (symbLoation <= token.Text.size())
		errorStream << token.Text[symbLoation];
	errorStream << "\" found" << " (line: " << token.Line << ", position: " << (token.Position + symbLoation) << ")";
	throw std::runtime_error(errorStream.str());
}

void	ErrorDispatcher::ValidateArgCount(const DirectiveUnit& conf, TokenSequence& seq)
{
	std::ostringstream errorStream;

	if (seq.Arguments.size() >= conf.MinArg && seq.Arguments.size() <= conf.MaxArg)
		return;
	if (seq.Arguments.size() > conf.MaxArg && conf.MaxArg == INFINIT_CONF_COUNT)
		return;

	int	position = seq.Directive.Position + seq.Directive.Text.size();
	if (!seq.Arguments.empty())
		position = seq.Arguments.back().Position;

	if (seq.Arguments.size() < conf.MinArg)
		errorStream << "Too few arguments for directive:";
	else if (seq.Arguments.size() > conf.MaxArg)
	{
		errorStream << "Invalid argument: \"" << seq.Arguments[conf.MaxArg].Text << "\"\n";
		errorStream << "Too many arguments for directive:";
	}

	errorStream << " \"" << seq.Directive.Text << "\"";
	errorStream << " (line: " << seq.Directive.Line << ", position: " << position<< ")";
	errorStream << "\nThe accepted number of arguments is ";
	if (conf.MinArg == conf.MaxArg)
		errorStream << ": " << conf.MinArg;
	else if (conf.MaxArg == INFINIT_CONF_COUNT)
		errorStream << "at least: " << conf.MinArg;
	else
		errorStream << "in the range: (" << conf.MinArg << " to " << conf.MaxArg << ")";

	throw std::runtime_error(errorStream.str());
}

void	ErrorDispatcher::ValidateContextLevel(const DirectiveUnit& conf, TokenSequence& seq)
{
	std::ostringstream errorStream;

	if (seq.CtxLevel >= conf.MinCtxLvl && seq.CtxLevel <= conf.MaxCtxLvl)
		return;
	if (seq.CtxLevel > conf.MaxCtxLvl && conf.MaxCtxLvl == INFINIT_CONF_COUNT)
		return;

	errorStream << "The directive: \"" << seq.Directive.Text << "\" is not allowed ";
	if (seq.CtxLevel > 1)
		errorStream << "inside the \"" << seq.ContextDirective.Text << "\" directive";
	else
		errorStream << "at this location:";

	int position = seq.Directive.Position + seq.Directive.Text.size();
	errorStream << " (line: " << seq.Directive.Line << ", position: " << position << ")";

	throw std::runtime_error(errorStream.str());
}

void	ErrorDispatcher::ValidateDelimiter(const DirectiveUnit& conf, TokenSequence& seq)
{
	std::ostringstream errorStream;

	if (seq.Delimiter.Text == conf.RequiredDelimiter)
		return;

	errorStream << "The directive \"" << seq.Directive.Text;
	if (seq.Delimiter.Text == "}")
		errorStream << "\" has an unexpected \"}\"";
	else if (conf.RequiredDelimiter == "{")
		errorStream << "\" is missing an opening \"{\"";
	else
		errorStream << "\" is not terminated by \"" << conf.RequiredDelimiter << "\"";
	int position = seq.Directive.Position + seq.Directive.Text.size();
	errorStream << " (line: " << seq.Directive.Line << ", position: " << position << ")";
	throw std::runtime_error(errorStream.str());
}

bool	ErrorDispatcher::ValidateUnknownDirective(DirectiveMap& map, TokenSequence& seq)
{
	std::ostringstream errorStream;

	if (map.find(seq.Directive.Text) == map.end())
	{
		if (seq.Directive.Text.empty() && seq.Arguments.empty() && !seq.Delimiter.Text.empty())
		{
			if (seq.Delimiter.Text == "}")
				return (false);
			errorStream << "Unexpected delimiter: \"" << seq.Delimiter.Text << "\" encountered.";
			errorStream << " (line: " << seq.Delimiter.Line << ", position: " << seq.Delimiter.Position << ")";
			throw std::runtime_error(errorStream.str());
		}
		ThrowUnknownDirectiveException(seq);
	}
	return (true);
}

void	ErrorDispatcher::ValidateEndOfFile(TokenSequence& seq, int finalContextlevel, int finalLine)
{
	std::ostringstream errorStream;

	if (!seq.IsDefault())
	{
		errorStream << "Unexpected end of file: expected \"}\" or \";\"";
		if (seq.Directive.Text.empty())
			errorStream << " (line: " << finalLine << ")";
		else
			errorStream << " (line: " << seq.Directive.Line << ")";
		throw std::runtime_error(errorStream.str());
	}
	if (finalContextlevel != 1)
	{
		errorStream << "Unexpected end of file: ";
		if (finalContextlevel > 1)
			errorStream << "missing closing \"}\"";
		else
			errorStream << "missing opening \"{\"";
		errorStream << " (line: " << finalLine << ")";
		throw std::runtime_error(errorStream.str());
	}
}

void	ErrorDispatcher::ValidateDuplicateCount(DirectiveUnit& conf, TokenSequence& seq)
{
	static DuplicationMap	dupMap;
	std::ostringstream		errorStream;

	// Wipe out any higher context levels because they are out of scope
	for (DuplicationMapIter outerIter =  dupMap.begin(); outerIter != dupMap.end(); ++outerIter)
	{
		if (outerIter->first > (size_t)seq.CtxLevel)
			outerIter->second.clear();
	}
	
	size_t dupCount = ++dupMap[seq.CtxLevel][seq.Directive.Text];
	if (conf.MaxDup == INFINIT_CONF_COUNT || (size_t)conf.MaxDup >= dupCount)
		return;
	errorStream << "The directive: \"" << seq.Directive.Text << "\" has exceeded the maximum allowed duplication count.";
	errorStream << " (line: " << seq.Directive.Line << ", position: " << seq.Directive.Position << ")";
	errorStream << "\nThe allowed maximum duplication count is: " << conf.MaxDup << ".";

	throw std::runtime_error(errorStream.str());
}

#pragma endregion

/// *** Operators *** ///
#pragma region Operators

ErrorDispatcher& ErrorDispatcher::operator=(const ErrorDispatcher& other)
{
	if (this != &other)
	{
		
	}
	return *this;
}

#pragma endregion
