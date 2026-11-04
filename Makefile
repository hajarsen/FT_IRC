NAME = ircserv

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -O3 -Iinclude -Inetwork -Ibonus
HEADERS = bonus/bonus.hpp \
		include/Channel.hpp \
		include/ChannelMode.hpp \
		include/CommandHandler.hpp \
		include/CommandParser.hpp \
		include/Response.hpp \
		include/User.hpp \
		network/Client.hpp \
		network/Server.hpp

SRCS = main.cpp \
		network/Client.cpp \
		network/Server.cpp \
		core/CommandHandler.cpp \
		bonus/bonus.cpp \
		core/CommandState.cpp \
		parsing/CommandParser.cpp \
		core/Response.cpp \
		core/ChannelMode.cpp \
		commands/Authentication.cpp \
		commands/Invite.cpp \
		commands/Join.cpp \
		commands/Kick.cpp \
		commands/Mode.cpp \
		commands/Part.cpp \
		commands/Ping.cpp \
		commands/Privmsg.cpp \
		commands/Quit.cpp \
		commands/Topic.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
.SECONDARY: $(OBJS)
