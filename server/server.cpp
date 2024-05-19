#include "server.hpp"

Server::Server( int port, std::string password )
{
	this->port = port;
	this->password = password;

	if ((this->fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	{
		ft_error_msg("Error!\nsocket");
		throw(std::exception());
	}

	int bool_value = 1;
	if (setsockopt( this->fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &bool_value, sizeof(bool_value)) != 0)
	{
		ft_error_msg("Error!\nsetsockopt");
		throw(std::exception());
	}

	memset(&(this->addr), 0, sizeof(this->addr));
	this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->addr.sin_port = htons(port);

	if (bind(this->fd, (struct sockaddr*) &(this->addr), sizeof(this->addr)) < 0)
	{
		ft_error_msg("Error!\nbind");
		throw(std::exception());
	}
	if (listen(this->fd, MAX_USERS) < 0) // MAX_USERS ?
	{
		close(this->fd);
		ft_error_msg("Error!\nlisten!");
		throw(std::exception());
	}

	struct pollfd pollfd;

	pollfd.fd = this->fd;
	pollfd.events = POLLIN;
	pollfd.revents = 0;
	this->fds.push_back(pollfd);

	std::cout << "Listening on port [" << this->port << "]\n";
}

Server::~Server( void )
{
	close(fd);
}

void Server::check_timeout(void)
{
	for (long unsigned int i = 1; i < this->fds.size(); ++i)
	{
		Client *client = this->client[i - 1];
		
		if (client->getIs_pong() == false)
		{

			std::vector<Channel *>::iterator it = this->getChannel().begin();
			std::vector<Channel *>::iterator ite = this->getChannel().end();

			while (it != ite)
			{
				if ((*it)->checkActive_users(client))
					(*it)->removeUser(client);
				it++;
			}

			close(this->fds[i].fd);

			std::cout << "Client Disconnected : "<< \
			" Client = " << client->getFd() << std::endl;

			delete client;

			this->fds.erase(this->fds.begin() + i);
			this->client.erase(this->client.begin() + i - 1);
		}
		else
		{
			client->setIs_pong(false);
			std::string msg = "PING " + client->getNickname() + "\n";
			send(client->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
		}
	}
}

int Server::call_poll( void )
{
	int poll_answer = poll(this->fds.data(), this->fds.size(), 3000);
	if (poll_answer > 0)
	{
		if (this->fds[0].revents & POLLIN) // Connexion d'un nouveau client
		{
			struct sockaddr_in client_addr;
			socklen_t addr_len = sizeof(client_addr);

			memset(&client_addr, 0, sizeof(client_addr));	
			int client_socket = accept(this->fds[0].fd, (struct sockaddr*) &client_addr, &addr_len);
			if (client_socket < 0)
				std::cout << "New connection refused\n";
			else
			{
				Client *client = new Client(client_socket, client_addr);

				if (client == NULL)
				{
					close (client_socket);
					return(ft_error_msg("Error!\nallocation client"));
				}

				this->client.push_back(client);
				
				pollfd pollfd;
				pollfd.fd = client_socket;
				pollfd.events = POLLIN;
				pollfd.revents = 0;
				this->fds.push_back(pollfd);
			
				std::cout << "New connection accepted :" << \
				" Client = " << client_socket << "\n";
			}
		}
		for (long unsigned int i = 1; i < this->fds.size(); ++i) // Traitement des clients
		{
			Client *client = this->client[i - 1];
			
			if (this->fds[i].revents & POLLIN) // en attente de lecture
			{
				int ret = client->recv_cmd(this);

				if (ret == 1)
					return (1);
				else if (ret == 2)
				{
					std::vector<Channel *>::iterator it = this->getChannel().begin();
					std::vector<Channel *>::iterator ite = this->getChannel().end();

					while (it != ite)
					{
						if ((*it)->checkActive_users(client))
							(*it)->removeUser(client);
						it++;
					}

					close(this->fds[i].fd);

					std::cout << "Client Disconnected : "<< \
					" Client = " << client->getFd() << std::endl;

					delete client;

					this->fds.erase(this->fds.begin() + i);
					this->client.erase(this->client.begin() + i - 1);
				}
			}
		}
	}
	else if (poll_answer == 0)
		this->check_timeout();
	else
		return(ft_error_msg("Error!\npoll"));
	return(0);
}

bool Server::getClient_status(std::string name){
	for (std::vector<Client *>::iterator it = this->client.begin(); it != this->client.end(); it++){
		if ((*it)->getNickname() == name)
			return true;
	}
	return false;
}

bool Server::getChannel_status(std::string chan){
	for (std::vector<Channel *>::iterator it = this->channel.begin(); it != this->channel.end(); it++){
		if ((*it)->getName() == chan)
			return true;
	}
	return false;
}

void Server::createChannel(std::string chan, std::string key, Client *client){
	Channel *c = new Channel(chan, key);
	c->addUser(client);
	c->addOperator(client);
	this->channel.push_back(c);
}

std::string Server::getPassword( void )
{
	return this->password;
}

int	Server::getPort( void )
{
	return this->port;
}

void Server::print(std::string msg, std::string client){
    for (std::vector<Client *>::iterator it = this->client.begin(); it != this->client.end(); it++){
        if ((*it)->getNickname() == client)
            send((*it)->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
    }
} 


std::vector<Channel *> &Server::getChannel(){return this->channel;}

std::vector<Client *> &Server::getClient(){return this->client;}

void Server::clearMemory(){
	for (std::vector<Channel *>::iterator it = this->channel.begin(); it != this->channel.end(); it++)
		(*it)->clearMemory();
	for (std::vector<Client *>::iterator it = this->client.begin(); it != this->client.end(); it++)
		(*it)->clearMemory();
	delete this;

}