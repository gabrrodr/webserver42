RED		:= \033[0;31m
GREEN	:= \033[0;32m
RESET	:= \033[0m


NAME 	= webserv
SRC_DIR	= source/
INC		= include/
OBJ_DIR = bin/
SRCS	=	$(addprefix $(SRC_DIR), main.cpp \
									Log.cpp \
									SimpleSocket.cpp \
									BindingSocket.cpp \
									ListeningSocket.cpp \
									ConnectionSocket.cpp \
									Config.cpp \
									Request.cpp)
OBJ		= $(patsubst $(SRC_DIR)%.cpp, $(OBJ_DIR)%.o, $(SRCS)) 
FLAGS	= -Wall -Wextra -Werror -std=c++98 -g

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	c++ $(FLAGS) -c $< -o $@
	@echo "[${GREEN}compiling${END}]: $<"

all: $(NAME)

$(OBJ_DIR):
	mkdir bin/
	@echo "${GREEN}bin created${END}"

$(NAME): $(OBJ)
	c++ $(FLAGS) -I $(INC) $(OBJ) -o $(NAME)
	@echo "${GREEN}${NAME} executable created${END}"

clean:
	@rm -rf ${OBJ_DIR}
	@echo "${RED}cleaning objects${END}"

fclean: clean
	@rm -rf ${NAME}
	@rm -f upload/upload
	@echo "${RED}${NAME} deleted${END}"

re: fclean all

.PHONY: all clean fclean re