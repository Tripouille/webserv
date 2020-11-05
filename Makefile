# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2020/11/05 15:27:30 by frfrey            #+#    #+#              #
#    Updated: 2020/11/05 18:02:48 by frfrey           ###   ########lyon.fr    #
#                                                                              #
# **************************************************************************** #


HEADER					= frfrey/includes/

SRC_PATH				= frfrey/srcs/

SRC_NAME				= tcp_server.cpp

SRCS					= $(addprefix $(SRC_PATH), $(SRC_NAME))

OBJ_NAME				= $(SRC_NAME:.cpp=.o)

OBJ_PATH				= obj/

OBJ						= $(addprefix $(OBJ_PATH), $(OBJ_NAME))

NAME					= server

RM						= rm -rf

CC						= clang++-9

FLAG					= -Wall -Wextra -Werror -std=c++17

ifdef DEBUG
FLAG					+= -g
endif

#FLAGS					= -g3 -fsanitize=address -Wconversion

# **************************************************************************** #
#								REGLES									       #
# **************************************************************************** #

all: $(OBJ_PATH) $(NAME) $(HEADER)

$(NAME): $(OBJ)
	@$(CC) $(FLAG) $(OBJ) -o $(NAME)
	@printf "\33[2K\r \033[0m\033[0;33mTcp_Server: \t\033[0;38;5;121mUpdated\n\033[0m"

$(OBJ_PATH):
	@mkdir -p obj/ 2> /dev/null

$(OBJ_PATH)%.o: $(SRC_PATH)%.cpp
	@printf "\033[2K\r\033[0;32m[Compiling] : \t\033[0;32m\033[0m$<"
	@$(CC) $(FLAG) -I $(HEADER) -c $< -o $@

clean:
	@printf "\033[2K\r\033[31mDeleting LeTcp_Serverarn srcs/	\033[37m"
	@sleep 0.1
	@printf "\033[2K\r\033[31mDeleting Tcp_Server srcs/.	\033[37m"
	@sleep 0.1
	@printf "\033[2K\r\033[31mDeleting Tcp_Server srcs/..	\033[37m"
	@sleep 0.1
	@printf "\033[2K\r\033[31mDeleting Tcp_Server srcs/...	\033[37m"
	@sleep 0.1
	@printf "\033[2K\r\033[31mDeleting Tcp_Server srcs/	\033[37m"
	@sleep 0.1
	@printf "\033[2K\r\033[31mDeleting Tcp_Server srcs/.	\033[37m"
	@sleep 0.1
	@printf "\033[2K\r\033[31mDeleting Tcp_Server srcs/..	\033[37m"
	@sleep 0.1
	@printf "\033[2K\r\033[31mDeleting Tcp_Server srcs/...	\033[37m"
	@sleep 0.1
	@${RM} ${OBJ_PATH}
	@printf "\33[2K\r \033[0;32m[OK] \033[0m \033[36m Deleted successfully!\n\033[0m"

fclean: clean
	@${RM} ${NAME}

re: fclean all

.PHONY: clean fclean
