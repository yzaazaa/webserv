# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yzirri <yzirri@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/04/19 10:43:59 by yzirri            #+#    #+#              #
#    Updated: 2024/09/20 09:23:16 by yzirri           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# General Varriables
START = '\e[1;94m'
END = '\e[0m'
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

NAME = webserv
OBJ_DIR = objects

MANDATORY_FILES = Mandatory/main.cpp 
MANDATORY_HEADER_FILES = 

PARSER_FILES = Parser/Parser.cpp Parser/Tokenizer.cpp Parser/Validator.cpp Parser/ErrorDispatcher.cpp \
						Parser/DirectiveMapper.cpp
PARSER_HEADER_FILES = Parser/Parser.hpp Parser/Tokenizer.hpp Parser/Validator.hpp Parser/ErrorDispatcher.hpp \
						Parser/DirectiveMapper.hpp

SERVER_FILES = Server/Server.cpp Server/ServerInitializer.cpp
SERVER_HEADER_FILES = Server/Server.hpp Server/ServerInitializer.hpp

CLIENT_FILES = Client/Client.cpp
CLIENT_HEADER_FILES = Client/Client.hpp

SERVER_INStANCE_FILES = 
SERVER_INSTANCE_HEADER_FILES = ServerInstance/ServerInstance.hpp

DEBUG_FILES = Debug/Debug.cpp
DEBUG_HEADER_FILES = Debug/Debug.hpp

SRC_FILES = $(MANDATORY_FILES) $(CLIENT_FILES) $(PARSER_FILES) $(SERVER_FILES) $(SERVER_INStANCE_FILES) $(DEBUG_FILES)

HEADER_FILES = $(MANDATORY_HEADER_FILES) $(CLIENT_HEADER_FILES) $(PARSER_HEADER_FILES) $(SERVER_HEADER_FILES) $(SERVER_INSTANCE_HEADER_FILES) $(DEBUG_HEADER_FILES)

OBJs = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

all : $(NAME)

$(NAME) : $(OBJs)
	$(CC) $(CFLAGS) $(OBJs) -o $(NAME)

$(OBJ_DIR)/%.o : %.cpp $(HEADER_FILES)
	mkdir -p $(dir $@)
	printf $(START)"[%-37s] 🕝 \r"$(END) "Compiling $(notdir $@)"
	$(CC) $(CFLAGS) -o $@ -c $<

clean :
	rm -rf $(OBJ_DIR)

fclean : clean
	rm -rf $(NAME)

re : fclean all
