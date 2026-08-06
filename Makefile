NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes

OBJ_DIR = .obj

SOURCES =	srcs/main.cpp srcs/Client.cpp srcs/Command.cpp \
			srcs/Server.cpp srcs/ServerSocket.cpp \
			srcs/ServerSend.cpp srcs/ServerCommand.cpp

OBJECTS = $(SOURCES:srcs/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)

$(OBJ_DIR)/%.o: srcs/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
