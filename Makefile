HEADERS_PATH			= includes/
HEADERS_NAME			= TcpListener.hpp
HEADERS					= $(addprefix $(HEADERS_PATH), $(HEADERS_NAME))

SRCS_PATH				= srcs/
SRCS_NAME				= main.cpp TcpListener.cpp
SRCS					= $(addprefix $(SRCS_PATH), $(SRCS_NAME))

OBJS_PATH				= objs/
OBJS					= $(addprefix $(OBJS_PATH), $(SRCS_NAME:.cpp=.o))

NAME					= webserv
RM						= rm -rf
CC						= clang++
FLAGS					= -Wall -Wextra -Werror -Wconversion -std=c++98

ifdef d
FLAGS					+= -g3
endif


all: $(NAME)

$(NAME): $(OBJS_PATH) $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

$(OBJS_PATH):
	mkdir -p objs

$(OBJS): objs/%.o: srcs/%.cpp $(HEADERS)
	$(CC) $(FLAGS) -I $(HEADERS_PATH) -c $< -o $@

clean:
	${RM} ${OBJS_PATH}

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: clean fclean
