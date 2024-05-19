SRCS		=	main.cpp \
				server/server.cpp \
				client/client.cpp \
				channel/channel.cpp \
				cmd/cmd.cpp \
				misc/misc.cpp \


OBJS		=	$(SRCS:.cpp=.o)

CXX			=	c++

CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98

RM			=	rm -rf

NAME		=	ircserv

#rules

all : $(NAME)

$(NAME) :  $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean :
	$(RM) $(OBJS) $(OBJS_B)

fclean : clean
	$(RM) $(NAME)

re : fclean all