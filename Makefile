# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aalleman <aalleman@student.42lyon.fr>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2020/11/05 15:27:30 by frfrey            #+#    #+#              #
#    Updated: 2020/11/25 16:56:43 by aalleman         ###   ########lyon.fr    #
#                                                                              #
# **************************************************************************** #


HEADERS_PATH			= includes/
HEADERS_NAME			= TcpListener.hpp
HEADERS					= $(addprefix $(HEADERS_PATH), $(HEADERS_NAME))

SRCS_PATH				= srcs/
SRCS_NAME				= main.cpp TcpListener.cpp
SRCS					= $(addprefix $(SRCS_PATH), $(SRCS_NAME))

OBJS_PATH				= objs/
OBJS_NAME				= $(SRCS_NAME:.cpp=.o)
OBJS					= $(addprefix $(OBJS_PATH), $(OBJS_NAME))

NAME					= webserv

RM						= rm -rf

CC						= clang++

FLAGS					= -Wall -Wextra -Werror -Wconversion -std=c++98

ifdef DEBUG
FLAGS					+= -g
endif

#FLAG					= -g3 -fsanitize=address

# **************************************************************************** #
#								REGLES									       #
# **************************************************************************** #

all: $(NAME)

$(NAME): $(OBJS_PATH) $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

$(OBJS_PATH):
	mkdir -p objs/ 2> /dev/null

$(OBJS): $(SRCS) $(HEADERS)
	$(CC) $(FLAGS) -I $(HEADERS_PATH) -c $< -o $@

clean:
	${RM} ${OBJS_PATH}

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: clean fclean
