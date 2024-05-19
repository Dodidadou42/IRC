#include "channel.hpp"

Channel::~Channel(){}
Channel::Channel(std::string chan, std::string key) : name(chan){
	if (key != ""){
		this->key = key;
		this->key_set = true;
	} else
		this->key_set = false;
	this->user_limit_set = false;
	this->whitelist_set = false;
	this->topic_restrict = false;
}


void Channel::print(std::string msg, Client *client){
	send(client->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
}
void Channel::print(std::string msg, std::string client){
	for (std::vector<Client *>::iterator it = this->active_users.begin(); it != this->active_users.end(); it++){
		if ((*it)->getNickname() == client)
			send((*it)->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
	}
} 
void Channel::printAll(std::string msg){
	for (std::vector<Client *>::iterator it = this->active_users.begin(); it != this->active_users.end(); it++)
		send((*it)->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
}
void Channel::printAllExceptClient(std::string msg, Client *client){
	for (std::vector<Client *>::iterator it = this->active_users.begin(); it != this->active_users.end(); it++)
	{
		if ((*it)->getNickname() != client->getNickname())
			send((*it)->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
	}
}
void Channel::printTopic(Client *client){
	std::string msg;
	if (!this->topic[0])
		msg = ":" + client->getServer_name() + " 331 " + client->getNickname() + " " + this->getName() + " :No topic is set\n";
	else
		msg = ":" + client->getServer_name() + " 332 " + client->getNickname() + " " + this->getName() + " " + this->getTopic() + "\n";
	this->print(msg, client);
}
void Channel::printModes(Client *client){
    std::string msg = ":" + client->getServer_name() + " 324 " + client->getNickname() + " " + this->getName() + " +";
    if (this->getWhitelist_set())
        msg += "i";
    if (this->getTopic_restrict())
        msg += "t";
    if (this->getKey_set())
        msg += "k";
    if (this->getUser_limit_set())
        msg += "l";
    if (this->getKey_set())
        msg += " " + this->getKey();
    if (this->getUser_limit_set()){
        std::stringstream ss;
        ss << this->getUserLimit();
        msg += " " + ss.str();
    }
    msg += "\n";
    this->print(msg, client);
}

void Channel::addUser(Client *client){
	if (!this->checkActive_users(client))
	{
		this->active_users.push_back(client);
		std::string msg = ":" + client->getNickname() + " JOIN " + this->getName() + "\n";
		this->printAll(msg);
		this->printTopic(client);
	}
}

void Channel::removeUser(Client *client){
	std::vector<Client *>::iterator it = this->active_users.begin();
	while (client->getNickname() != (*it)->getNickname())
		it++;
	std::string msg = ":" + client->getNickname() + " PART " + this->getName() + "\n";
	this->printAll(msg);
	if (this->checkOperators(client))
	{
		std::vector<Client *>::iterator it_operators = this->operators.begin();
		while (client->getNickname() != (*it_operators)->getNickname())
			it_operators++;
		this->operators.erase(it_operators);
	}
	if (this->checkWhitelist(client))
	{
		std::vector<Client *>::iterator it_whitelist = this->whitelist.begin();
		while (client->getNickname() != (*it_whitelist)->getNickname())
			it_whitelist++;
		this->whitelist.erase(it_whitelist);
	}
	this->active_users.erase(it);
}

void Channel::kickUser(Client *client, std::string to_kick, std::string reason)
{
	std::vector<Client *>::iterator it = this->active_users.begin();
	std::vector<Client *>::iterator ite = this->active_users.end();
	while (it != ite && to_kick != (*it)->getNickname())
		it++;
	if (it == ite)
		return ;
	std::string msg;
	if (reason != "")
		msg = ":" + client->getNickname() + " KICK " + this->getName() + " " + (*it)->getNickname() + " :" + reason + "\n";
	else
		msg = ":" + client->getNickname() + " KICK " + this->getName() + " " + (*it)->getNickname() + "\n";
	this->printAll(msg);
	//REMOVE FROM OPERATORS
	std::vector<Client *>::iterator it_op = this->operators.begin();
	while (it_op != this->operators.end()){
		if (to_kick == (*it_op)->getNickname())
			break;
		it_op++;
	}
	if (it_op != this->operators.end())
		this->operators.erase(it_op);
	//REMOVE FROM WHITELIST
	std::vector<Client *>::iterator it_wh = this->whitelist.begin();
	while (it_wh != this->whitelist.end()){
		if (to_kick == (*it_wh)->getNickname())
			break;
		it_wh++;
	}
	if (it_wh != this->whitelist.end())
		this->whitelist.erase(it_wh);
	//REMOVE FROM CHANNEL
	this->active_users.erase(it);
}

void Channel::addOperator(Client *client){
	if (!this->checkOperators(client))
		this->operators.push_back(client);
	std::string msg = ":" + client->getNickname() + " MODE " + this->getName() + " +o " + client->getNickname() + "\n";
	this->printAll(msg);
}

void Channel::addOperator(std::string client, Client *c){
	if (!this->checkActive_users(client))
		return ;
	std::vector<Client *>::iterator it = this->active_users.begin();
	while (client != (*it)->getNickname())
		it++;
	if (!this->checkOperators(*it))
		this->operators.push_back(*it);
	std::string msg = ":" + c->getNickname() + " MODE " + this->getName() + " +o " + client + "\n";
	this->printAll(msg);
}

void Channel::removeOperator(std::string client, Client *c){

	std::vector<Client *>::iterator it = this->operators.begin();
	while (it != this->operators.end()){
		if (client == (*it)->getNickname())
			break;
		it++;
	}
	if (it != this->operators.end())
		this->operators.erase(it);
	std::string msg = ":" + c->getNickname() + " MODE " + this->getName() + " -o " + client + "\n";
	this->printAll(msg);

}

void Channel::addWhitelist(Client *client){
	if (!this->checkWhitelist(client))
		this->whitelist.push_back(client);
}
bool Channel::checkWhitelist(Client *client){
	for (std::vector<Client *>::iterator it = this->whitelist.begin(); it != this->whitelist.end(); it++){
		if (client->getNickname() == (*it)->getNickname())
			return true;
	}
	return false;
}
bool Channel::checkUser_limit(){
	if (this->active_users.size() >= (size_t)this->getUserLimit())
		return false;
	return true;
}
bool Channel::checkNo_users(){
	if (this->active_users.size() == 0)
		return true;
	return false;
}
bool Channel::checkActive_users(Client *client){
	for (std::vector<Client *>::iterator it = this->active_users.begin(); it != this->active_users.end(); it++){
		if (client->getNickname() == (*it)->getNickname())
			return true;
	}
	return false;
}
bool Channel::checkActive_users(std::string nick){
	for (std::vector<Client *>::iterator it = this->active_users.begin(); it != this->active_users.end(); it++){
		if (nick == (*it)->getNickname())
			return true;
	}
	return false;
}
bool Channel::checkOperators(Client *client){
	for (std::vector<Client *>::iterator it = this->operators.begin(); it != this->operators.end(); it++){
		if (client->getNickname() == (*it)->getNickname())
			return true;
	}
	return false;
}

void Channel::setTopic(Client *client, std::string topic){
	this->topic = topic;
	this->printAll(":" + client->getNickname() + " TOPIC " + this->getName() + " :" + topic + "\n");
}
void Channel::setWhitelist(bool b, Client *client){
	this->whitelist_set = b;
	std::string msg;
	if (b)
		msg = ":" + client->getNickname() + " MODE " + this->getName() + " +i\n";
	else
		msg = ":" + client->getNickname() + " MODE " + this->getName() + " -i\n";
	this->printAll(msg);
}

void Channel::setTopic_restrict(bool b, Client *client){
	this->topic_restrict = b;
	std::string msg;
	if (b)
		msg = ":" + client->getNickname() + " MODE " + this->getName() + " +t\n";
	else
		msg = ":" + client->getNickname() + " MODE " + this->getName() + " -t\n";
	this->printAll(msg);
}


void Channel::setKey(bool b, std::string key, Client *client){
	std::string msg;
	if (!b){
		this->key_set = false;
		msg = ":" + client->getNickname() + " MODE " + this->getName() + " -k\n";
	}
	else {
		this->key_set = true;
		this->key = key;
		msg = ":" + client->getNickname() + " MODE " + this->getName() + " +k " + key + "\n";
	}
	this->printAll(msg);
}
void Channel::setUser_limit(bool b, std::string lim, Client *client){
	std::string msg;
	if (!b){
		this->user_limit_set = false;
		msg = ":" + client->getNickname() + " MODE " + this->getName() + " -l\n";
	}
	else {
		for (unsigned int i = 0; i < lim.size(); i++){
			if (!isdigit(lim[i]))
				return;
		}
		if (std::atoi(lim.c_str()) < 0)
			return ;
		this->user_limit_set = true;
		this->user_limit = std::atoi(lim.c_str());
		msg = ":" + client->getNickname() + " MODE " + this->getName() + " +l " + lim + "\n";
	}
	this->printAll(msg);
}

int	Channel::getUserNumber()
{
	std::vector<Client *>::iterator it = this->active_users.begin();
	std::vector<Client *>::iterator ite = this->active_users.end();
	int i = 0;
	while (it != ite)
	{
		it++;
		i++;
	}
	return i;
}

std::vector<Client *> &Channel::getActive_users(){return this->active_users;}

std::string Channel::getName(){return this->name;}
std::string Channel::getKey(){return this->key;}
std::string Channel::getTopic(){return this->topic;}
int		 Channel::getUserLimit(){return this->user_limit;}
bool		Channel::getKey_set(){return this->key_set;}
bool		Channel::getUser_limit_set(){return this->user_limit_set;}
bool		Channel::getTopic_restrict(){return this->topic_restrict;}
bool		Channel::getWhitelist_set(){return this->whitelist_set;}

void Channel::clearMemory(){
	delete this;
}