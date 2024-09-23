/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveMapper.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 08:42:39 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/20 09:22:32 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
# include "Validator.hpp"
# include "Parser.hpp"

struct	TokenSequence;
struct	DirectiveUnit;
class	Parser;

typedef	std::map<std::string, DirectiveUnit>	DirectiveMap;
typedef	std::vector<TokenSequence>				SeqVector;
typedef	std::vector<TokenSequence>::iterator	SeqVectorIter;

class DirectiveMapper
{
	private:
		DirectiveMapper();
		DirectiveMapper(const DirectiveMapper& other);
		DirectiveMapper& operator=(const DirectiveMapper& other);

		// Functions
		/// @brief if Location/Server did not override a config it inhertis it from preveus context.
		static void SyncDefaultConfig(std::map<int, ServerInstance>& servers, const HttpInstance& httpInstance);
		/// @brief Assign source to target if the target is not defined.
		template <typename T>
		static void AssignIfNotDefined(T& target, const T& source, bool isDefined);
		/// @brief Load http configuration.
		static void HttpMapper(HttpInstance& httpInstance, SeqVectorIter& it);
		/// @brief Load Server configuration.
		static void	ServerMapper(SeqVectorIter& it, Parser& parser, SeqVector& tokenSeqances);
		/// @brief Load Location configuration.
		static void	LocationMapper(SeqVectorIter& it, ServerInstance& instance, SeqVector& tokenSeqances);
		/// @brief Parses the "return" validates it, and throws a descriptive exception on failure.
		static void	ParseRedirect(RedirectionData& redirectionData, TokenSequence seq);
		/// @brief Parses the "only_allow" validates it, and throws a descriptive exception on failure.
		static void ParseAllowedMethods(std::set<std::string>& parsedAllowedMethos, TokenSequence seq);
		/// @brief Parses the "server_name" validates it, and throws a descriptive exception on failure.
		static void	AddServerNames(ServerInstance& serverInstance, TokenSequence& seq);
		/// @brief Parses the "listen" validates it, and throws a descriptive exception on failure.
		static void	AddListeningSocket(ServerInstance& serverInstance, TokenSequence seq);
		/// @brief Parses the "error_page" validates it, and throws a descriptive exception on failure.
		static void	ParseErrorPage(ErrorPageData& parsedErrorPase, TokenSequence seq);
		/// @brief Parses the "client_max_body_size" validates it, and throws a descriptive exception on failure.
		static void	ParseBodySize(ClientMaxBodySize& parsedMaxBody, TokenSequence seq);
		/// @brief Parses the "root" validates it, and throws a descriptive exception on failure.
		static void	ParseRoot(RootDirectoryData& parsedMaxBody, TokenSequence seq);
		/// @brief Parses the "autoindex" validates it, and throws a descriptive exception on failure.
		static void	ParseAutoIndex(AutoIndexData& autoIndex, TokenSequence seq);
		/// @brief Parses the "index" validates it, and throws a descriptive exception on failure.
		static void	ParseIndex(IndexData& index, TokenSequence seq);
		/// @brief Parses the "cgi_pass" validates it, and throws a descriptive exception on failure.
		static void	ParseCgiPass(CgiPassData& cgiMap, TokenSequence seq);
	
	public:
		// Functions
		/// @brief Constructs configuration data for all supported directives, populating the directive map.
		static void	BuildDirectiveMap(DirectiveMap& dirMap);
		/// @brief Initiates the configuration mapping process using the parser and token sequences.
		static void	StartConfigMapping(Parser& parser, SeqVector& tokenSeqances);

};
