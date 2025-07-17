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
									Request.cpp \
									BufferRequest.cpp \
									CGIRequest.cpp \
									Signal.cpp \
									Utils.cpp)

OBJ = $(addprefix $(OBJ_DIR), $(notdir $(SRCS:.cpp=.o)))

FLAGS	= -Wall -Wextra -Werror -std=c++98 -g #-fsanitize=address

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
	@rm -f upload/upload.txt
	@echo "${RED}${NAME} deleted${END}"

re: fclean all

.PHONY: all clean fclean re