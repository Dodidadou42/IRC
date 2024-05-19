#include "client.hpp"

Client::Client( int fd, sockaddr_in addr)
{
	this->fd = fd;
	this->addr = addr;

	this->nick_status = 0;
	this->pass_status = 0;
	this->user_status = 0;

	this->nickname = "*";

	this->is_pong = true;
	this->is_disconnected = 0;
}

Client::~Client( void )
{
	return ;
}

int Client::getFd( void )
{
	return (this->fd);
}

int Client::recv_cmd( Server *server )
{
	(void) server;
	char buffer[BUFFER_SIZE];

	int read_recv = recv(this->fd, buffer, BUFFER_SIZE, 0);
	if (read_recv == 0) // la connexion a été fermée
		return (2);
	else if (read_recv < 0) // erreur de reception
		return (0);
	else
	{
		buffer[read_recv] = '\0';
		this->cmd_buffer += buffer;

		long unsigned int line_return = this->cmd_buffer.find("\n");

		while (line_return != std::string::npos)
		{
			std::string cmd_line = cmd_buffer.substr(0, line_return);
			if (cmd_line != "\r" && cmd_line != "")
			{
				Cmd *cmd = new Cmd(cmd_line, this, server);
				if (cmd == NULL)
					return(ft_error_msg("Error!\nallocation cmd"));

				cmd->ft_analyse_cmd();
				delete cmd;
				
				if (this->is_disconnected == 1)
					return (2);
			}

			this->cmd_buffer.erase(0, line_return + 1);
			line_return = this->cmd_buffer.find("\n");
		}
	}
	return (0);
}

void Client::setServer_name(std::string server_name){
	this->server_name = server_name;
}

void Client::setTried_nickname(std::string nick){
	this->tried_nickname = nick;
}

void Client::setPassword(std::string pass){
	this->password = pass;
}

void Client::setUsername(std::string user){
	this->username = user;
}

void Client::setNickname(std::string nick){
	this->nickname = nick;
}

void Client::setRealname(std::string realname){
	this->realname = realname;
}

void Client::setHost(std::string host){
	this->host = host;
}

std::string Client::getServer_name( void ){
	return this->server_name;
}

std::string Client::getTried_nickname( void )
{
	return this->tried_nickname;
}

std::string Client::getPassword( void )
{
	return this->password;
}

std::string Client::getUsername( void )
{
	return this->username;
}

std::string Client::getNickname( void )
{
	return this->nickname;
}

std::string Client::getRealname( void )
{
	return this->realname;
}

std::string Client::getHost( void ){
	return this->host;
}

void Client::setPass_status( int new_status )
{
	this->pass_status = new_status;
}

void Client::setNick_status( int new_status )
{
	this->nick_status = new_status;
}

void Client::setUser_status( int new_status )
{
	this->user_status = new_status;
}

int Client::getPass_status( void )
{
	return (this->pass_status);
}

int Client::getNick_status( void )
{
	return (this->nick_status);
}

int Client::getUser_status( void )
{
	return (this->user_status);
}

void Client::setIs_disconnected( int i )
{
	this->is_disconnected = i;
}

int Client::getIs_disconnected( void )
{
	return (this->is_disconnected);
}

void Client::setIs_pong( bool b )
{
	this->is_pong = b;
}

int Client::getIs_pong( void )
{
	return (this->is_pong);
}

void Client::clearMemory(){
	delete this;
}