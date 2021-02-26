HEADERS_PATH			= includes/
HEADERS_NAME			= ServerConfig.hpp TcpListener.hpp HttpRequest.hpp CgiRequest.hpp \
						BufferQ.hpp Client.hpp Answer.hpp base64.hpp md5.hpp MultiServ.hpp \
						utils.hpp Regex.hpp thread.hpp
HEADERS					= $(addprefix $(HEADERS_PATH), $(HEADERS_NAME))

SRCS_PATH				= srcs/
SRCS_NAME				= main.cpp ServerConfig.cpp TcpListener.cpp HttpRequest.cpp CgiRequest.cpp \
						BufferQ.cpp Client.cpp Answer.cpp base64.cpp md5.cpp MultiServ.cpp utils.cpp \
						Regex.cpp thread.cpp
SRCS					= $(addprefix $(SRCS_PATH), $(SRCS_NAME))

OBJS_PATH				= objs/
OBJS					= $(addprefix $(OBJS_PATH), $(SRCS_NAME:.cpp=.o))

NAME					= webserv
RM						= rm -rf
CC						= clang++
FLAGS					= -Wall -Wextra -Werror -std=c++11 -fsanitize=address

ifdef DEBUG
FLAGS					+= -g3
endif

test: all
	./webserv

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
