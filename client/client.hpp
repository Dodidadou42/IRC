#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "../ircserv.hpp"

class Server;
class Cmd;
class Client {

public:

	Client( int fd, sockaddr_in addr );
	~Client( void );

	int recv_cmd( Server *server );

	void setTried_nickname(std::string pass);
	void setPassword(std::string pass);
	void setUsername(std::string user);
	void setNickname(std::string nick);
	void setRealname(std::string realname);
	void setServer_name(std::string server_name);
	void setHost(std::string host);

	std::string getTried_nickname( void );
	std::string getPassword( void );
	std::string getUsername( void );
	std::string getNickname( void );
	std::string getRealname( void );
	std::string getServer_name( void );
	std::string getHost( void );

	void setPass_status( int new_status );
	void setNick_status( int new_status );
	void setUser_status( int new_status );
		
	int getFd( void );
	int getPass_status( void );
	int getNick_status( void );
	int getUser_status( void );

	void setIs_disconnected( int i );
	int getIs_disconnected( void );

	void setIs_pong( bool b );
	int getIs_pong( void );

	void clearMemory();

private:

	Client( void );
	Client( Client const & src );
	Client & operator=( Client const & rhs );

	int  fd;
	struct 	sockaddr_in addr;

	std::string cmd_buffer;

	int pass_status;
	int nick_status;
	int user_status;

	int is_pong;
	int is_disconnected;

	std::string tried_nickname;
	std::string password;
	std::string nickname;
	std::string username;
	std::string realname;
	std::string server_name;
	std::string host;
};

#endif