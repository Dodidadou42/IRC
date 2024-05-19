#ifndef SERVER_HPP
# define SERVER_HPP

# include "../ircserv.hpp"
# include "../channel/channel.hpp"

class Client;
class Cmd;
class Channel;
class Server {

	public:

		Server( int port, std::string password );
		~Server( void );

		void check_timeout(void);

		int call_poll( void );

		bool	getChannel_status(std::string name);
		bool	getClient_status(std::string name);
		void	createChannel(std::string channel, std::string key, Client *client);

		std::string getPassword( void );

		std::vector<Channel *> &getChannel();
		std::vector<Client *> &getClient();

		int		getPort( void );

		void	print(std::string msg, std::string client);
		void	clearMemory();

	private:

		Server( void );
		Server( Server const & src );
		Server & operator=( Server const & rhs );

		int         port;
		std::string password;

		int         fd;
		struct      sockaddr_in addr;

		std::vector<Client *> client;
		std::vector<Channel *> channel;

		std::vector<pollfd> fds;

};

#endif