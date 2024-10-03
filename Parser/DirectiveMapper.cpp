/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveMapper.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 08:42:35 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/20 09:20:56 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "DirectiveMapper.hpp"

/// @brief Make sure they are uppercase
const char*	Methods[] = { "GET", "POST" };
const int	MethodsLen = sizeof(Methods) / sizeof(Methods[0]);

/// *** Constructors *** ///
#pragma region Constructors

DirectiveMapper::DirectiveMapper()
{

}

DirectiveMapper::DirectiveMapper(const DirectiveMapper& other)
{
	*this = other;
}

#pragma endregion

/// *** Functions *** ///
#pragma region Functions

static void ThrowInvalidValueError(const std::string& directive, const std::string& value, const std::string& mustBe, int line)
{
	std::ostringstream errorStream;

	errorStream << "Invalid value: \"" << value << "\" in the \"";
    errorStream << directive << "\" directive.";
	if (!mustBe.empty())
    	errorStream << " " << mustBe << ".";
    errorStream << "\nIn (line: " << line << ").";
	throw std::runtime_error(errorStream.str());
}

void	DirectiveMapper::ParseCgiPass(CgiPassData& cgiMap, TokenSequence seq)
{
	if (seq.Directive.Text != "cgi_pass")
		return;
	std::string cgiExtention = seq.Arguments[0].Text;
	std::string cgiPath = seq.Arguments[1].Text;

	if (cgiPath.empty() || cgiPath.empty())
		ThrowInvalidValueError(seq.Directive.Text, cgiPath, "It must not be empty", seq.Directive.Line);
	cgiMap.Value[cgiExtention] = cgiPath;
	cgiMap.IsDefined = true;
}

void	DirectiveMapper::ParseIndex(IndexData& index, TokenSequence seq)
{
	if (seq.Directive.Text != "index")
		return;
	for (TokenIter it = seq.Arguments.begin(); it != seq.Arguments.end(); ++it)
	{
		Token& token = *it;
		if (token.Text.size() <= 0)
			ThrowInvalidValueError(seq.Directive.Text, token.Text, "It must not be empty", seq.Directive.Line);
		index.Index.insert(token.Text);
		index.IsDefined = true;
	}
}

void	DirectiveMapper::ParseAutoIndex(AutoIndexData& autoIndex, TokenSequence seq)
{
	if (seq.Directive.Text != "autoindex")
		return;
	std::string value = seq.Arguments[0].Text;
	for (size_t i = 0; i < value.size(); i++)
		value[i] = std::toupper(value[i]);
	
	if (value == "ON")
		autoIndex = AutoIndexData(true);
	else if (value == "OFF")
		autoIndex = AutoIndexData(false);
	else
		ThrowInvalidValueError(seq.Directive.Text, value, "It must be \"on\" or \"off\"", seq.Directive.Line);
}

void	DirectiveMapper::ParseRoot(RootDirectoryData& parsedMaxBody, TokenSequence seq)
{
	if (seq.Directive.Text != "root")
		return;
	parsedMaxBody = RootDirectoryData(seq.Arguments[0].Text);
}

void	DirectiveMapper::ParseBodySize(ClientMaxBodySize& parsedMaxBody, TokenSequence seq)
{
	if (seq.Directive.Text != "client_max_body_size")
		return;
	
	std::string input = seq.Arguments[0].Text;
	size_t numericEnd = 0;

	if (input.empty())
		ThrowInvalidValueError(seq.Directive.Text, input, "It must not be empty", seq.Directive.Line);

	while (numericEnd < input.size() && std::isdigit(input[numericEnd]))
		numericEnd++;
	if (numericEnd <= 0)
		ThrowInvalidValueError(seq.Directive.Text, input, "Accepted format is number/unit", seq.Directive.Line);
	else
	{
		std::string valueString = input.substr(0, numericEnd);
		std::string unitString = input.substr(numericEnd);
		if (valueString.size() >= 13 || unitString.size() > 1)
			ThrowInvalidValueError(seq.Directive.Text, input, "Accepted format is number/unit", seq.Directive.Line);
		
		unitString[0] = std::tolower(unitString[0]);
		size_t result = std::stoul(valueString);
		if (unitString[0] == 'b' || unitString.size() <= 0)
			parsedMaxBody = ClientMaxBodySize(result, 0, 0, 0);
		else if (unitString[0] == 'k')
			parsedMaxBody = ClientMaxBodySize(0, result, 0, 0);
		else if (unitString[0] == 'm')
			parsedMaxBody = ClientMaxBodySize(0, 0, result, 0);
		else if (unitString[0] == 'g')
			parsedMaxBody = ClientMaxBodySize(0, 0, 0, result);
		else
			ThrowInvalidValueError(seq.Directive.Text, input, "Invalid unit: only 'b', 'k', 'm', or 'g' are accepted", seq.Directive.Line);
	}
}

void	DirectiveMapper::ParseErrorPage(ErrorPageData& parsedErrorPase, TokenSequence seq)
{
	if (seq.Directive.Text != "error_page")
		return;

	std::string path = seq.Arguments.back().Text;
	seq.Arguments.pop_back();

	for (TokenIter it = seq.Arguments.begin(); it != seq.Arguments.end(); ++it)
	{
		std::string errorCode = it->Text;
		if (errorCode.size() <= 0)
			ThrowInvalidValueError(seq.Directive.Text, errorCode, "It must not be empty", seq.Directive.Line);

		for (size_t i = 0; i < errorCode.size(); i++)
		{
			if (!std::isdigit(errorCode[0]))
				ThrowInvalidValueError(seq.Directive.Text, errorCode, "", seq.Directive.Line);
		}
		size_t errorCodeValue = std::stoul(errorCode);
		if (errorCodeValue < 300 || errorCodeValue > 599)
			ThrowInvalidValueError(seq.Directive.Text, errorCode, "Value must be between 300 and 599", seq.Directive.Line);
		parsedErrorPase.AddEntry(errorCodeValue, path);
	}
}

void	DirectiveMapper::AddListeningSocket(ServerInstance& serverInstance, TokenSequence seq)
{
	if (seq.Directive.Text != "listen")
		return;
	std::string input = seq.Arguments[0].Text;
	ListenInfo listenInfo;
	listenInfo.Port = input;
	listenInfo.Address = "";

	size_t dotsPosition = input.find(':');
	if (dotsPosition != std::string::npos)
	{
		listenInfo.Address  = input.substr(0, dotsPosition);
		listenInfo.Port = input.substr(dotsPosition + 1);
	}

	//TODO: u sure?
	std::pair<std::set<ListenInfo>::iterator, bool> result = serverInstance.ServerSockets.insert(listenInfo);
	if (!result.second)
	{
		std::string error = "Parse error: Duplicate listen [" + listenInfo.ToString() + "] in server number: ";
		error += std::to_string(serverInstance.ServerIndex + 1);
		throw std::runtime_error(error);
	}
}

void	DirectiveMapper::AddServerNames(ServerInstance& serverInstance, TokenSequence& seq)
{
	if (seq.Directive.Text != "server_name")
		return;
	for (TokenIter it = seq.Arguments.begin(); it != seq.Arguments.end(); ++it)
		serverInstance.ServerNames.insert(it->Text);
}

void DirectiveMapper::ParseAllowedMethods(std::set<std::string>& parsedAllowedMethos, TokenSequence seq)
{
	if (seq.Directive.Text != "only_allow")
		return;
	for (TokenIter it = seq.Arguments.begin(); it != seq.Arguments.end(); ++it)
	{
		for (int i = 0; i < MethodsLen; i++)
		{
			for (size_t j = 0; j < it->Text.size(); j++)
				it->Text[j] = std::toupper(it->Text[j]);
			if (it->Text == std::string(Methods[i]))
			{
				parsedAllowedMethos.insert(it->Text);
				break;
			}
			if (i + 1 >= MethodsLen)
				ThrowInvalidValueError(seq.Directive.Text, it->Text, "invalid method", seq.Directive.Line);
		}
	}
}

void	DirectiveMapper::ParseRedirect(RedirectionData& redirectionData, TokenSequence seq)
{
	if (seq.Directive.Text != "return")
		return;
	std::string code = seq.Arguments[0].Text;
	std::string value = seq.Arguments[1].Text;
	for (size_t i = 0; i < code.size(); i++)
	{
		if (std::isdigit(code[i]))
			continue;
		ThrowInvalidValueError(seq.Directive.Text, code, "", seq.Directive.Line);
	}
	size_t codeValue = std::stoul(code);
	if (codeValue < 100 || codeValue > 599)
		ThrowInvalidValueError(seq.Directive.Text, code, "Value must be between 100 and 599", seq.Directive.Line);
	redirectionData = RedirectionData(codeValue, value);
}

void	DirectiveMapper::LocationMapper(SeqVectorIter& it, ServerInstance& instance, SeqVector& tokenSeqances)
{
	std::ostringstream errorStream;
	bool	isExactMatch = false;
	std::string path = it->Arguments[0].Text;

	if (it->Arguments.size() > 1)
	{
		if (it->Arguments[0].Text.size() != 1 || it->Arguments[0].Text[0] != '=')
		{
			errorStream << "Parse error: Invalid location modifier \"" << it->Arguments[0].Text << "\"";
			errorStream << "\nIn (line: " << it->Directive.Line << ").";
			throw std::runtime_error(errorStream.str());
		}
		isExactMatch = true;
		path = it->Arguments[1].Text;
	}

	LocationInstance location(isExactMatch, path);
	if (instance.Locations.find(location.PathArg) != instance.Locations.end())
	{
		errorStream << "Parse error: Duplicate location \"" << location.PathArg << "\"";
		errorStream << "\nIn (line: " << it->Directive.Line << ").";
		throw std::runtime_error(errorStream.str());
	}

	int ctxLvl = it->CtxLevel;
	++it;
	for (; it != tokenSeqances.end(); ++it)
	{
		TokenSequence& seq = *it;
		if (seq.CtxLevel <= ctxLvl)
			break;
		if (seq.Directive.Text.empty())
			continue;
		ParseBodySize(location.ClientMaxBody, seq);
		ParseErrorPage(location.ErrorPages, seq);
		ParseAllowedMethods(location.AllowedMethods, seq);
		ParseRedirect(location.Redirection, seq);
		ParseRoot(location.RootDirectory, seq);
		ParseAutoIndex(location.IsAutoIndex, seq);
		ParseIndex(location.IndexValue, *it);
		ParseCgiPass(location.CgiConfig, *it);
	}
	instance.Locations[location.PathArg] = location;
}

void	DirectiveMapper::ServerMapper(SeqVectorIter& it, Parser& parser, SeqVector& tokenSeqances)
{
	int				ctxLvl;
	static int		serverIndex = 0;

	if (it->Directive.Text != "server")
		return;
	parser.GetParsedServers()[serverIndex] = ServerInstance(serverIndex);
	ServerInstance& serverInstance = parser.GetParsedServers()[serverIndex++];
	ctxLvl = it->CtxLevel;
	it++;

	// load config
	for (; it != tokenSeqances.end(); ++it)
	{
		TokenSequence& seq = *it;
		if (seq.CtxLevel <= ctxLvl)
			break;
		AddListeningSocket(serverInstance, seq);
		AddServerNames(serverInstance, seq);
		ParseBodySize(serverInstance.ClientMaxBody, seq);
		ParseErrorPage(serverInstance.ErrorPages, seq);
		ParseRoot(serverInstance.RootDirectory, seq);
		ParseAutoIndex(serverInstance.IsAutoIndex, seq);
		ParseIndex(serverInstance.IndexValue, seq);
		ParseCgiPass(serverInstance.CgiConfig, seq);
		if (it->Directive.Text == "location")
			LocationMapper(it, serverInstance, tokenSeqances);
	}
}

void DirectiveMapper::HttpMapper(HttpInstance& httpInstance, SeqVectorIter& it)
{
	ParseCgiPass(httpInstance.CgiPass, *it);
	ParseIndex(httpInstance.Index, *it);
	ParseRoot(httpInstance.Root, *it);
	ParseBodySize(httpInstance.MaxBodySize, *it);
	ParseErrorPage(httpInstance.ErrorPages, *it);
	ParseAutoIndex(httpInstance.AutoIndex, *it);
}

template <typename T>
void DirectiveMapper::AssignIfNotDefined(T& target, const T& source, bool isDefined)
{
    if (!isDefined)
        target = source;
}

void DirectiveMapper::SyncDefaultConfig(std::map<int, ServerInstance>& servers, const HttpInstance& httpInstance)
{
	for (ServerInstanceIterator serverIt = servers.begin(); serverIt != servers.end(); ++serverIt)
	{
		ServerInstance& instance = serverIt->second;
		AssignIfNotDefined(instance.ClientMaxBody, httpInstance.MaxBodySize, instance.ClientMaxBody.IsDefined);
		AssignIfNotDefined(instance.ErrorPages, httpInstance.ErrorPages, instance.ErrorPages.IsDefined);
		AssignIfNotDefined(instance.RootDirectory, httpInstance.Root, instance.RootDirectory.IsOverriten);
		AssignIfNotDefined(instance.IsAutoIndex, httpInstance.AutoIndex, instance.IsAutoIndex.IsDefined);
		AssignIfNotDefined(instance.IndexValue, httpInstance.Index, instance.IndexValue.IsDefined);
		AssignIfNotDefined(instance.CgiConfig, httpInstance.CgiPass, instance.CgiConfig.IsDefined);
		for (LocationsIter locIt = instance.Locations.begin(); locIt != instance.Locations.end(); ++locIt)
		{
			LocationInstance& location = locIt->second;
			AssignIfNotDefined(location.ClientMaxBody, instance.ClientMaxBody, location.ClientMaxBody.IsDefined);
			AssignIfNotDefined(location.ErrorPages, instance.ErrorPages, location.ErrorPages.IsDefined);
			AssignIfNotDefined(location.RootDirectory, instance.RootDirectory, location.RootDirectory.IsOverriten);
			AssignIfNotDefined(location.IsAutoIndex, instance.IsAutoIndex, location.IsAutoIndex.IsDefined);
			AssignIfNotDefined(location.IndexValue, instance.IndexValue, location.IndexValue.IsDefined);
			AssignIfNotDefined(location.CgiConfig, instance.CgiConfig, location.CgiConfig.IsDefined);
		}
	}
}

void DirectiveMapper::StartConfigMapping(Parser& parser, SeqVector& tokenSeqances)
{
	SeqVectorIter	innerIt;
	HttpInstance	httpInstance;

	for (SeqVectorIter it =  tokenSeqances.begin(); it != tokenSeqances.end(); ++it)
	{
		HttpMapper(httpInstance, it);
		ServerMapper(it, parser, tokenSeqances);
	}
	SyncDefaultConfig(parser.GetParsedServers(), httpInstance);
}

void	DirectiveMapper::BuildDirectiveMap(DirectiveMap& dirMap)
{
	// everything
	dirMap["http"] = DirectiveUnit(1, 1, 1, "{", 0, 0);
	dirMap["error_page"] = DirectiveUnit(2, 4, INFINIT_CONF_COUNT, ";", 2, INFINIT_CONF_COUNT);
	dirMap["client_max_body_size"] = DirectiveUnit(2, 4, 1, ";", 1, 1);
	dirMap["root"] = DirectiveUnit(2, 4, 1, ";", 1, 1);
	dirMap["autoindex"] = DirectiveUnit(2, 4, 1, ";", 1, 1);
	dirMap["index"] = DirectiveUnit(2, 4, 1, ";", 1, INFINIT_CONF_COUNT);
	dirMap["cgi_pass"] = DirectiveUnit(2, 4, INFINIT_CONF_COUNT, ";", 2, 2);

	//just server
	dirMap["server"] = DirectiveUnit(2, 2, INFINIT_CONF_COUNT, "{", 0, 0);
	dirMap["listen"] = DirectiveUnit(3, 3, INFINIT_CONF_COUNT, ";", 1, 1); //server
	dirMap["server_name"] = DirectiveUnit(3, 3, INFINIT_CONF_COUNT, ";", 1, INFINIT_CONF_COUNT); //server

	// Just location
	dirMap["location"] = DirectiveUnit(3, 3, INFINIT_CONF_COUNT, "{", 1, 1); //server
	dirMap["only_allow"] = DirectiveUnit(4, 4, 1, ";", 1, INFINIT_CONF_COUNT);
	dirMap["return"] = DirectiveUnit(4, 4, 1, ";", 2, 2);
}

#pragma endregion

/// *** Operators *** ///
#pragma region Operators

DirectiveMapper& DirectiveMapper::operator=(const DirectiveMapper& other)
{
	if (this != &other)
	{
		
	}
	return *this;
}

#pragma endregion
