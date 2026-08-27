NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes
# CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes -g -O0 -fsanitize=address -fno-omit-frame-pointer

OBJ_DIR = .obj

SERVER_SOURCES = srcs/Server/Server.cpp \
                 srcs/Server/ServerCommand.cpp \
                 srcs/Server/ServerSend.cpp \
                 srcs/Server/ServerSocket.cpp \
                 srcs/Server/ServerUtils.cpp

COMMAND_SOURCES = srcs/Command/Command.cpp \
                  srcs/Command/Ping.cpp \
                  srcs/Command/Quit.cpp \
                  srcs/Command/Cap.cpp \
                  srcs/Command/Pass.cpp \
                  srcs/Command/Nick.cpp \
                  srcs/Command/User.cpp \
                  srcs/Command/Privmsg.cpp \
                  srcs/Command/Join.cpp \
                  srcs/Command/Topic.cpp \
                  srcs/Command/Kick.cpp \
                  srcs/Command/Invite.cpp \
                  srcs/Command/Mode.cpp \
                  srcs/Command/ModeApply.cpp

CORE_SOURCES = srcs/main.cpp \
               srcs/Client.cpp \
               srcs/ClientRegister.cpp \
               srcs/Channel.cpp

SOURCES = $(CORE_SOURCES) $(SERVER_SOURCES) $(COMMAND_SOURCES)

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
