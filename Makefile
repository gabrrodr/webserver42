RED		:= \033[0;31m
GREEN	:= \033[0;32m
RESET	:= \033[0m


NAME 	=	webserv
HEADERS	=	networking/MainHeader.hpp 

SRCS	=	networking/main.cpp \
			networking/Log.cpp \
			networking/SimpleSocket.cpp \
			networking/ConnectionSocket.cpp \
			networking/BindingSocket.cpp \
			networking/ListeningSocket.cpp \
			networking/Config.cpp \
			#networking/Utils.cpp \

OBJ = $(SRCS:.cpp=.o) 

all:
	@$(MAKE) $(NAME)


$(NAME) : $(OBJ) $(HEADERS)
	c++ -Wall -Wextra -Werror -std=c++98 $(OBJ) -o $(NAME)
	@echo -e "$(GREEN)-COMPILATION DONE... $(RESET)"

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -f $(NAME)
	@echo -e "$(RED)-WEBSERV GONE... $(RESET)"

re:		fclean all


.PHONY: all clean fclean re