/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Testing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/17 08:16:33 by yzirri            #+#    #+#             */
/*   Updated: 2024/09/18 06:08:41 by yzirri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <cctype>
# include <cstddef>
# include <iostream>
# include <ostream>

# include <string>
# include <vector>

// a word is anything connected and this is considered connected (hello"world to this"world)
// Comments

/*
	Rules
		1: Variable Keywords:
			.a line with a Varriable keyword must end with ';'
			.Varriable keywords can only exist in a single line and require ';' in that line
		2: Keywords
			.a Keyword must be followed with '{' and ends with a '}' before the next keyword or EOF
		3:General notes
			.the config file can be in a single line
		4: Comments
			.a comment starts with '#' and ends with EOF, anything after '#' and before EOF or newl ine is a comment
			.comments are considered non existent
		6: Strings
			.a string must be preceded by a space and start with a quotation mark,
			  if not its not considered a string and the string rules do not apply to it
				Space"Zoom" == Zoom BUT Zoo"m" == Zoo"m" and Zoom"" == Zoom""
				Space"Zoo"m == Error
			.a valid string must end with a quotation mark or ';' anything else is considered an error
			.strings back to back with diffrent types are not allowed
				"Zo""om" == error, 'Zo''om' = error,
				"Zo'om" == Zo'om, 'Zo"om' = Zo"om
			
*/

enum TokenType
{
	
	directive, directiveArguments,
	SectionKeyword,
	PropertyKeyword,
	PropertyValue,
	PropertyStringValue,
	PARAMETER,
	PARAMETER_STRING,
	Semicolon
};

struct Token
{
	std::string Text;
	int lineCount;
};

const char* Keywords[] = { "server", "listen", "server_name", "http" };
const int KeywordsLen = sizeof(Keywords) / sizeof(Keywords[0]);

static bool IsQuotationMark(char c) { return (c == '\'' || c == '\"'); }
std::string GetBoolString(bool bl) { return (bl ?  "True" : "False"); }
bool IsSemicolon(char c, bool isString) { return (c == ';' && !isString); }

bool IsSpace(char c, bool isString)
{
	return (std::isspace(c) && !isString);
}

static char quoteType;

bool	TokenizeLine(std::string& line, std::vector<Token>& tokens, int lineCount, bool isString)
{
	bool isLogging = false;
	for (size_t i = 0; i < line.size(); i++)
	{
		if (IsSpace(line[i], isString))
			continue;
		Token token;
		token.Text = "";
		token.lineCount = lineCount;
		bool isComment = !isString && line[i] == '#';
		if (isString && !tokens.empty())
		{
			token.Text = tokens.back().Text + '\n';
			tokens.pop_back();
		}
		
		if (!isComment && ((i > 0 && std::isspace(line[i - 1])) || i == 0))
		{
			if (!isString && IsQuotationMark(line[i]))
				quoteType = line[i];
			isString = isString ? true : IsQuotationMark(line[i]);
		}
		
		
		for (size_t j = i; j < line.size(); j++)
		{
			bool isLastElement = j + 1 >= line.size();
			if (line[j] == quoteType && i != j)
				isString = false;
			
			if (isString && !isLastElement && line[j + 1] != '\n')
				continue;
			else if (!isLastElement)
			{
				if (isComment)
					continue;
				if (!IsSpace(line[j], isString) && (line[j] != ';' && line[j + 1] != ';' ) && (line[j] != '{' && line[j + 1] != '{' ) && 
						(line[j] != '}' && line[j + 1] != '}' ))
					continue;
			}
			size_t startPosition = i;
			size_t endPosition = j;
			if (std::isspace(line[j]) && !isString)
				endPosition--;
			size_t length = endPosition - startPosition + 1;
			token.Text += line.substr(startPosition, length);
			
			tokens.push_back(token);
			i = j;
			break;
		}
	}

	return (isString);
}

int main(int argc, char **argv)
{
	std::vector<Token> tokens;
	
	std::ifstream file(argv[1]);
	if (!file.is_open())
        return (std::cerr << "Error: Could not open file." << std::endl, 1);
    std::string line;
	int lineCount = 1;
	bool isString = false;
    while (std::getline(file, line))
		isString = TokenizeLine(line, tokens, lineCount++, isString);


	file.close();

	
	std::vector<Token>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); ++it)
	{
		Token& token = *it;
		std::cout << "Token: [" << token.Text << "]"/*, Size: "*/ << std::endl;
	}
}