#ifndef CMD_HPP
# define CMD_HPP

# include "../ircserv.hpp"

class Server;
class Client;
class Cmd {

	public:

		Cmd( std::string cmd, Client * client, Server *server );
		~Cmd( void );

		void ft_analyse_cmd( void );

	private:

		Cmd( void );
		Cmd( Cmd const & src );
		Cmd & operator=( Cmd const & rhs );

		void pass( void );

		int  is_valid_nickname( std::string nickname );
		int  is_available_nickname( std::string nickname );
		void nick( void );

		void user( void );
		void quit( void );
		void parse_join( void );
		void join(std::string channel, std::string key);
		void parse_part( void );
		void part( std::string channel );
		void parse_mode( void );
		void mode(bool b, std::string mode);
		void topic( void );
		void parse_names( void );
		void names( std::string channel );
		void parse_list( void );
		void list( std::string channel );
		void invite( void );
		void kick( void );
		void parse_privmsg( void );
		void privmsg( std::string dest );
		void notice( void );
		void who( void );
		void cap( void );
		void pong( void );

		void errorMsg(std::string code, std::string cmd, std::string what);
		void replyMsg(std::string code, std::string cmd, std::string what);

		int getIs_last_param( void );

		Client *client;
		Server *server;
		
		std::string cmd;
		std::vector<std::string> args;
		std::string last_param;
		
		int is_last_param;
};

#endif