NAME = webserv

SRCS = Client.cpp main.cpp Request.cpp

HEADERS = Client.hpp Request.hpp

OBJS = ${SRCS:.cpp=.o}

FLAGS = -Wall -Wextra -Werror

COMP = c++

all : $(NAME)

$(NAME) : $(OBJS)
	$(COMP) $(OBJS) -o $(NAME)

%.o : %.cpp $(HEADERS)
	$(COMP) $(FLAGS) -c $< -o $@

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -rf $(NAME)

re : fclean all

.PHONY : clean