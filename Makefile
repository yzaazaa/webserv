START = '\e[1;94m'
END = '\e[0m'
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

NAME = webserv
OBJ_DIR = z_objects

MANDATORY_FILES = Mandatory/main.cpp
MANDATORY_HEADER_FILES = 

PARSER_FILES = Parser/Parser.cpp Parser/Tokenizer.cpp Parser/Validator.cpp Parser/ErrorDispatcher.cpp \
						Parser/DirectiveMapper.cpp
PARSER_HEADER_FILES = Parser/Parser.hpp Parser/Tokenizer.hpp Parser/Validator.hpp Parser/ErrorDispatcher.hpp \
						Parser/DirectiveMapper.hpp

SERVER_FILES = Server/Server.cpp Server/ServerInitializer.cpp
SERVER_HEADER_FILES = Server/Server.hpp Server/ServerInitializer.hpp

SERVER_INStANCE_FILES = 
SERVER_INSTANCE_HEADER_FILES = ServerInstance/ServerInstance.hpp

DEBUG_FILES = Debug/Debug.cpp
DEBUG_HEADER_FILES = Debug/Debug.hpp

KQUEUE_UTILS_FILES = KqueueUtils/KqueueUtils.cpp
KQUEUE_UTILS_HEADER_FILES = KqueueUtils/KqueueUtils.hpp

HTTP_FILES = Http/Client.cpp \
			Http/HeaderValidator/HeaderValidator.cpp \
			Http/ResponseUtils/ResponseUtils.cpp \
			Http/FileUtils.cpp
HTTP_HEADER_FILES = Http/Client.hpp Http/Request.hpp Http/Response.hpp \
					Http/HeaderValidator/HeaderValidator.hpp \
					Http/ResponseUtils/ResponseUtils.hpp \
					Http/FileUtils.hpp

CGI_FILES = CGI/Cgi.cpp

CGI_HEADER_FILES = CGI/Cgi.hpp


SRC_FILES = $(MANDATORY_FILES) $(PARSER_FILES) $(SERVER_FILES) $(SERVER_INStANCE_FILES) $(DEBUG_FILES) \
				$(KQUEUE_UTILS_FILES) $(HTTP_FILES) $(CGI_FILES)

HEADER_FILES = $(MANDATORY_HEADER_FILES) $(PARSER_HEADER_FILES) $(SERVER_HEADER_FILES) $(SERVER_INSTANCE_HEADER_FILES) \
				$(DEBUG_HEADER_FILES) $(KQUEUE_UTILS_HEADER_FILES) $(HTTP_HEADER_FILES) $(CGI_HEADER_FILES)

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

.PHONY : clean fclean all

.SILENT : $(NAME) clean fclean all ${OBJs}