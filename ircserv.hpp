#ifndef IRCSERV_HPP
# define IRCSERV_HPP

# include <iostream> // cout
# include <sstream> // iss
# include <cstdlib> // atoi
# include <csignal> //signal

# include <string.h> // memset

# include <unistd.h> // close

# include <sys/socket.h> // socket
# include <arpa/inet.h> // sockaddr
# include <poll.h> // poll

# include <vector> // vector
# include <map> // map

# include "server/server.hpp"
# include "client/client.hpp"
# include "cmd/cmd.hpp"
# include "misc/misc.hpp"

# define BUFFER_SIZE 512
# define MAX_USERS 1000

extern Server *g_server;

#endif