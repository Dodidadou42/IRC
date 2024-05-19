#include "cmd.hpp"

Cmd::Cmd( std::string cmd, Client * client, Server * server )
{
	this->client = client;
	this->server = server;

	std::size_t i = cmd.find(':', 0);

	if (i != std::string::npos)
	{
		is_last_param = 1;
		this->last_param = cmd.substr(i + 1);
	}
	else
	{
		is_last_param = 0;
		last_param = "";
	}

	std::stringstream ss(cmd.substr(0, i));
	std::string word;
	while (ss >> word)
		this->args.push_back(word);

	this->cmd = this->args[0];
	this->args.erase(this->args.begin());

	std::cout << cmd << std::endl;
}

Cmd::~Cmd( void ) 
{
	return ;
}

void Cmd::ft_analyse_cmd( void )
{
	typedef void (Cmd::*CmdPTR)();
	CmdPTR	cmd_function[] = {&Cmd::pass, &Cmd::nick, &Cmd::user, \
	&Cmd::quit, &Cmd::parse_join, &Cmd::parse_part, &Cmd::parse_mode, \
	&Cmd::topic, &Cmd::parse_names, &Cmd::parse_list, &Cmd::invite, \
	&Cmd::kick, &Cmd::parse_privmsg, &Cmd::notice, &Cmd::who, &Cmd::cap, &Cmd::pong};
	std::string	cmd_list[] = {"PASS", "NICK", "USER", "QUIT", \
	"JOIN", "PART", "MODE", "TOPIC", "NAMES", "LIST", "INVITE", \
	"KICK", "PRIVMSG", "NOTICE", "WHO", "CAP", "PONG"};

	int	i = -1;
	while (++i < 17)
	{
		if (cmd_list[i] == this->cmd)
		{
			(this->*cmd_function[i])();
			return ;
		}
	}
	// 421 ERR_UNKNOWNCOMMAND
	return this->errorMsg("421", this->cmd, "Unknown command");
}

void Cmd::pass( void ) 
{
	if (this->args.size() < 1) // 461 ERR_NEEDMOREPARAMS
		return this->errorMsg("461", this->cmd, "Not enough parameters");
	else if (this->client->getUser_status() == 1) // 462 ERR_ALREADYREGISTRED
		return this->errorMsg("462", "", "You may not register");
	this->client->setPassword(this->args[0]);
	this->client->setPass_status(1);
}

int Cmd::is_valid_nickname( std::string nickname )
{
	if (!(('a' <= nickname[0] && nickname[0] <= 'z') || ('A' <= nickname[0] && nickname[0] <= 'Z')))
		return (0);
	int i = 0;
	while (nickname[++i])
	{
		if (!(('a' <= nickname[i] && nickname[i] <= 'z') || ('A' <= nickname[i] && nickname[i] <= 'Z') \
		|| ('0' <= nickname[i] && nickname[i] <= '9') || nickname[i] == '-' || nickname[i] == '[' \
		|| nickname[i] == ']' || nickname[i] == '\\' || nickname[i] == '`' || nickname[i] == '^' \
		|| nickname[i] == '{' || nickname[i] == '}'))
			return (0);
	}
	return (1);
}

int Cmd::is_available_nickname( std::string nickname )
{
	std::vector<Client *> client = server->getClient();
	std::vector<Client *>::iterator it = client.begin();
	std::vector<Client *>::iterator ite = client.end();

	while (it != ite)
	{
		if ((*it)->getNickname() == nickname)
			return (0);
		it++;
	}
	return (1);

}

void Cmd::nick( void )
{
	if (!this->client->getPass_status()) // Password not registered => Ignore
		return ;
	if (this->args.size() < 1) // 431 ERR_NONICKNAMEGIVEN
	{
		if (this->client->getTried_nickname() == "")
			this->client->setTried_nickname(this->args[0]);
		return this->errorMsg("431", "", "No nickname given");
	}
	if (this->is_valid_nickname(this->args[0]) == 0) // 432 ERR_ERRONEUSNICKNAME
	{
		if (this->client->getTried_nickname() == "")
			this->client->setTried_nickname(this->args[0]);
		return this->errorMsg("432", this->args[0], "Erroneus nickname");
	}
	if (this->is_available_nickname(this->args[0]) == 0) // 433 ERR_NICKNAMEINUSE
	{
		if (this->client->getTried_nickname() == "")
			this->client->setTried_nickname(this->args[0]);
		return this->errorMsg("433", this->args[0], "Nickname is already in use");
	}

	std::string answer;
	if (this->client->getNickname() == "") // New user
	{
		if (this->client->getTried_nickname() == "") // Directly a correct nickname
			answer = ":" + this->args[0] + " NICK " + this->args[0] + "\n";
		else // First try failled
			answer = ":" + this->client->getTried_nickname() + " NICK " + this->args[0] + "\n";
	}
	else // User already nick
		answer = ":" + this->client->getNickname() + " NICK " + this->args[0] + "\n";

	this->client->setNickname(this->args[0]);
	std::vector<Channel *>::iterator it = this->server->getChannel().begin();
	std::vector<Channel *>::iterator ite = this->server->getChannel().end();
	while (it != ite)
	{
		if ((*it)->checkActive_users(this->client))
			(*it)->printAll(answer);
		it++;
	}
	send(this->client->getFd(), answer.c_str(), sizeof(const char) * answer.length(), 0);

	if (this->client->getUser_status() == 1 && this->client->getNick_status() == 0)
	{
		if (this->client->getPassword() != this->server->getPassword())
		{
			this->errorMsg("464", "", "Password incorrect");
			this->client->setIs_disconnected(1);
		}
		else
			this->replyMsg("001", "", "Welcome on this IRCserver");
	}
	this->client->setNick_status(1);
}

void Cmd::user( void )
{
	if (!this->client->getPass_status()) // Password not registered => Ignore
		return ;
	if (this->args.size() < 3 || this->is_last_param == 0) // 461 ERR_NEEDMOREPARAMS
		return this->errorMsg("461", this->cmd, "Not enough parameters");
	if (this->client->getUser_status() == 1) // 462 ERR_ALREADYREGISTRED
		return this->errorMsg("462", "", "You may not register");
	this->client->setUsername(this->args[0]);
	this->client->setHost(this->args[1]);
	this->client->setServer_name(this->args[2]);
	this->client->setRealname(this->last_param);

	if (this->client->getUser_status() == 0 && this->client->getNick_status() == 1)
	{
		if (this->client->getPassword() != this->server->getPassword())
		{
			this->errorMsg("464", "", "Password incorrect");
			this->client->setIs_disconnected(1);
		}
		else
			this->replyMsg("001", "", "Welcome on this IRCserver");
	}
	this->client->setUser_status(1);
}

void Cmd::quit( void )
{
	if (this->client->getUser_status() && this->client->getNick_status())
	{
		std::string answer = ":" + this->client->getNickname() + " QUIT :" + this->last_param + "\n";
		send(this->client->getFd(), answer.c_str(), sizeof(const char) * answer.length(), 0);
	}
	this->client->setIs_disconnected(1);
	return ;
}

void Cmd::join(std::string chan, std::string key){
	if (!this->client->getUser_status() || !this->client->getNick_status())
		return;
	if (chan.empty() || chan[0] != '#' || chan.length() == 1) //403 ERR_NOSUCHCHANNEL
		return this->errorMsg("403", chan, "No such channel");
	if (!this->server->getChannel_status(chan))
		return this->server->createChannel(chan, key, this->client);
	std::vector<Channel *>::iterator it = this->server->getChannel().begin();
	while ((*it)->getName() != chan)
		it++;
	if ((*it)->getWhitelist_set() && !(*it)->checkWhitelist(this->client)) //473 ERR_INVITEONLYCHAN
		return this->errorMsg("473", chan, "Cannot join channel (+i)");
	if ((*it)->getKey_set() && (*it)->getKey() != key) //475 ERR_BADCHANNELKEY
		return this->errorMsg("475", chan, "Cannot join channel (+k)");
	if ((*it)->getUser_limit_set() && !(*it)->checkUser_limit()) //471 ERR_CHANNELISFULL
		return this->errorMsg("471", chan, "Cannot join channel (+l)");
	if ((*it)->checkActive_users(this->client))
		return ;
	(*it)->addUser(this->client);
}

void Cmd::parse_join( void )
{
	if (this->args.size() < 1) // 461 ERR_NEEDMOREPARAMS
		return this->errorMsg("461", this->cmd, "Not enough parameters");
	std::string chan, key, c = this->args[0], k = this->args[1];
	while (1){
		chan = c.substr(0, c.find_first_of(','));
		key = k.substr(0, k.find_first_of(','));
		if (c.find_first_of(',') == std::string::npos)
			c.clear();
		else
			c.erase(0, c.find_first_of(',') + 1);
		if (k.find_first_of(',') == std::string::npos)
			k.clear();
		else
			k.erase(0, k.find_first_of(',') + 1);
		if (chan != key)
			this->join(chan, key);
		else
			this->join(chan, "");
		if (c.empty())
			break;
	}
}

void Cmd::parse_part( void )
{
	if (this->args.size() < 1) // 461 ERR_NEEDMOREPARAMS
		return this->errorMsg("461", this->cmd, "Not enough parameters");
	std::string channel;
	std::string channel_list = this->args[0];
	while (1){
		channel = channel_list.substr(0, channel_list.find_first_of(','));
		if (channel_list.find_first_of(',') == std::string::npos)
			channel_list.clear();
		else
			channel_list.erase(0, channel_list.find_first_of(',') + 1);
		this->part(channel);
		if (channel_list.empty())
			break;
	}
}

void Cmd::part( std::string channel )
{
	if (!this->client->getUser_status() || !this->client->getNick_status())
		return;
	if (channel.empty() || channel[0] != '#') //403 ERR_NOSUCHCHANNEL
		return this->errorMsg("403", channel, "No such channel");
	std::vector<Channel *>::iterator it = this->server->getChannel().begin();
	std::vector<Channel *>::iterator ite = this->server->getChannel().end();
	while (it != ite && (*it)->getName() != channel)
		it++;
	if (it == ite) //403 ERR_NOSUCHCHANNEL
		return this->errorMsg("403", channel, "No such channel");
	if ((*it)->checkActive_users(this->client) == false) // 442 ERR_NOTONCHANNEL
		return this->errorMsg("442", channel, "You're not on that channel");
	(*it)->removeUser(this->client);
	if ((*it)->checkNo_users())
	{
		delete (*it);
		this->server->getChannel().erase(it);
	}
}
void Cmd::mode(bool b, std::string mode){
	std::vector<Channel *>::iterator it = this->server->getChannel().begin();
	while ((*it)->getName() != this->args[0])
		it++;
	std::string mod = (b) ? "+" : "-";
	int n = 0;

	for (unsigned int i = 0; i < mode.size(); i++){
		switch (mode[i]){
			case 'i':
				(*it)->setWhitelist(b, this->client);
				break;
			case 't':
				(*it)->setTopic_restrict(b, this->client);
				break;
			case 'k':
				if (b){
					(*it)->setKey(b, this->args[2 + n], this->client);
					n++;
				} else
					(*it)->setKey(b, "", this->client);
				break;
			case 'o':
				if (b)
					(*it)->addOperator(this->args[2 + n], this->client);
				else 
					(*it)->removeOperator(this->args[2 + n], this->client);
				n++;
				break;
			case 'l':
				if (b){
					(*it)->setUser_limit(b, this->args[2 + n], this->client);
					n++;
				}
				else
					(*it)->setUser_limit(b, "", this->client);
				break;
		}
	}
}

void Cmd::parse_mode( void )
{ 
    if (this->args.size() < 1) // 461 ERR_NEEDMOREPARAMS
		return this->errorMsg("461", this->cmd, "Not enough parameters");
	if (!this->client->getUser_status() || !this->client->getNick_status()) //Start Channel
		return ;
	if (!this->server->getChannel_status(this->args[0])) //403 ERR_NOSUCHCHANNEL
		return this->errorMsg("403", this->args[0], "No such channel");
    std::vector<Channel *>::iterator it = this->server->getChannel().begin();
    while ((*it)->getName() != this->args[0])
		it++;
	if (!(*it)->checkActive_users(this->client)) //442 ERR_NOTONCHANNEL
		return this->errorMsg("442", this->args[0], "You're not on that channel");
	if (this->args.size() == 1)
	{
		(*it)->printModes(this->client);
		this->names(this->args[0]);
		this->replyMsg("366", this->args[0], "End of /NAMES list");
		return ;
	}
	if (!(*it)->checkOperators(this->client)) //482 ERR_CHANOPRIVSNEEDED
		return this->errorMsg("482", this->args[0], "You're not channel operator");
	if (this->args[1][0] != '+' && this->args[1][0] != '-') //472 ERR_UNKNOWNMODE
		return this->errorMsg("472", this->args[1].substr(0, 1), "is unknown mode char to me");
	for (unsigned int i = 1; i < this->args[1].size(); i++){
		if (this->args[1][i] != 'i' && this->args[1][i] != 't' && this->args[1][i] != 'k' && this->args[1][i] != 'o' && this->args[1][i] != 'l')
			return this->errorMsg("472", this->args[1].substr(i, 1), "is unknown mode char to me");
		if (this->args[1][i] == 'k' && this->args[1][0] == '+' && this->args.size() < 3) // 461 ERR_NEEDMOREPARAMS
			return this->errorMsg("461", this->cmd, "Not enough parameters");
		if (this->args[1][i] == 'o' && this->args.size() < 3) // 461 ERR_NEEDMOREPARAMS
			return this->errorMsg("461", this->cmd, "Not enough parameters");
		if (this->args[1][i] == 'l' && this->args[1][0] == '+' && this->args.size() < 3) // 461 ERR_NEEDMOREPARAMS
			return this->errorMsg("461", this->cmd, "Not enough parameters");
	}
	if (this->args[1][0] == '+' && this->args[1].find('k') != std::string::npos && this->args[1].find('l') != std::string::npos && this->args.size() < 4)
		return this->errorMsg("461", this->cmd, "Not enough parameters");
	bool b = (this->args[1][0] == '+');
	this->mode(b, this->args[1].substr(1));
}

void Cmd::topic( void )
{
	if (this->args.size() < 1) // 461 ERR_NEEDMOREPARAMS
		return this->errorMsg("461", this->cmd, "Not enough parameters");
	if (!this->server->getChannel_status(this->args[0]))
		return ;
	if (!this->client->getUser_status() || !this->client->getNick_status())
		return;
	std::vector<Channel *>::iterator it = this->server->getChannel().begin();
	while ((*it)->getName() != this->args[0])
		it++;
	if (!(*it)->checkActive_users(this->client)) //442 ERR_NOTONCHANNEL
		return this->errorMsg("442", this->args[0], "You're not on that channel");
	if (this->getIs_last_param() == 0) 
		(*it)->printTopic(this->client);
	else
	{
		if ((*it)->getTopic_restrict() && !(*it)->checkOperators(this->client)) //482 ERR_CHANOPRIVSNEEDED
			return this->errorMsg("482", this->args[0], "You're not channel operator");
		(*it)->setTopic(this->client, this->last_param);
	}
}

void Cmd::parse_names( void )
{
	if (!this->client->getUser_status() || !this->client->getNick_status())
		return;
	std::string first_channel = "";
	if (this->args.size() < 1)
	{
		std::vector<Channel *>::iterator it = this->server->getChannel().begin();
		std::vector<Channel *>::iterator ite = this->server->getChannel().end();
		while (it != ite)
		{
			this->names((*it)->getName());
			it++;
		}
	}
	else
	{
		int first = 1;
		std::string channel;
		std::string channel_list = this->args[0];
		while (1){
			channel = channel_list.substr(0, channel_list.find_first_of(','));
			if (channel_list.find_first_of(',') == std::string::npos)
				channel_list.clear();
			else
				channel_list.erase(0, channel_list.find_first_of(',') + 1);
			if (first == 1)
			{
				first = 0;
				first_channel = channel;
			}
			this->names(channel);
			if (channel_list.empty())
				break;
		}
	}
	this->replyMsg("366", first_channel, "End of /NAMES list");

}

void Cmd::names( std::string channel )
{
	std::vector<Channel *>::iterator it_channel = this->server->getChannel().begin();
	std::vector<Channel *>::iterator ite_channel = this->server->getChannel().end();
	while (it_channel != ite_channel)
	{
		if ((*it_channel)->getName() == channel)
			break;
		it_channel++;
	}
	if (it_channel == ite_channel)
		return;
	std::string msg = ":" + this->client->getServer_name() + " 353 " + this->client->getNickname() + " = " + channel + " :";
	std::vector<Client *>::iterator it_client = (*it_channel)->getActive_users().begin();
	std::vector<Client *>::iterator ite_client = (*it_channel)->getActive_users().end();
	while (it_client != ite_client)
	{
		if ((*it_channel)->checkOperators(*it_client))
			msg += "@";
		msg += (*it_client)->getNickname() + " ";
		it_client++;
	}
	msg += "\n";
	send(this->client->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
}

void Cmd::parse_list( void )
{
	if (!this->client->getUser_status() || !this->client->getNick_status())
		return;
	this->replyMsg("321", "Channel", "Users Name");
	if (this->args.size() < 1)
	{
		std::vector<Channel *>::iterator it = this->server->getChannel().begin();
		std::vector<Channel *>::iterator ite = this->server->getChannel().end();
		while (it != ite)
		{
			this->list((*it)->getName());
			it++;
		}
	}
	else
	{
		std::string channel;
		std::string channel_list = this->args[0];
		while (1){
			channel = channel_list.substr(0, channel_list.find_first_of(','));
			if (channel_list.find_first_of(',') == std::string::npos)
				channel_list.clear();
			else
				channel_list.erase(0, channel_list.find_first_of(',') + 1);
			this->list(channel);
			if (channel_list.empty())
				break;
		}
	}
	this->replyMsg("323", "", "End of /LIST");
}

void Cmd::list( std::string channel )
{
	std::vector<Channel *>::iterator it = this->server->getChannel().begin();
	std::vector<Channel *>::iterator ite = this->server->getChannel().end();
	while (it != ite)
	{
		if ((*it)->getName() == channel)
			break;
		it++;
	}
	if (it == ite)
		return;
	std::stringstream ss;
	ss << (*it)->getUserNumber();
	this->replyMsg("322", (*it)->getName() + " " + ss.str(), (*it)->getTopic());
}

void Cmd::invite( void )
{
	if (this->args.size() < 2) // 461 ERR_NEEDMOREPARAMS
		return this->errorMsg("461", this->cmd, "Not enough parameters");
	if (!this->server->getChannel_status(this->args[1])) //442 ERR_NOTONCHANNEL
		return this->errorMsg("442", this->args[1], "You're not on that channel");
	if (!this->client->getUser_status() || !this->client->getNick_status())
		return;
	std::vector<Channel *>::iterator it = this->server->getChannel().begin();
	while ((*it)->getName() != this->args[1])
		it++;
	if (!(*it)->checkActive_users(this->client)) //442 ERR_NOTONCHANNEL
		return this->errorMsg("442", this->args[1], "You're not on that channel");
	if ((*it)->getWhitelist_set() && !(*it)->checkOperators(this->client)) //482 ERR_CHANOPRIVSNEEDED
		return this->errorMsg("482", this->args[1], "You're not channel operator");
	if (!this->server->getClient_status(this->args[0])) //401 ERR_NOSUCHNICK
		return this->errorMsg("401", this->args[0], "No such nick/channel");
	if ((*it)->checkActive_users(this->args[0])) //443 ERR_USERONCHANNEL
		return this->errorMsg("443", this->args[0] + " " + this->args[1], "is already on channel");
	std::vector<Client *>::iterator it_client = this->server->getClient().begin();
	while (1)
	{
		if ((*it_client)->getNickname() == this->args[0])
			break;
		it_client++;
	}
	(*it)->addWhitelist(*it_client);
	std::string msg = ":" + client->getNickname() + " " + this->cmd + " " + this->args[0] + " " + this->args[1] + "\n";
	this->replyMsg("341", this->args[0], this->args[1]);
	this->server->print(msg, this->args[0]);
}

void Cmd::kick( void )
{
	if (this->args.size() < 2) // 461 ERR_NEEDMOREPARAMS
		return this->errorMsg("461", this->cmd, "Not enough parameters");
	if (!this->client->getUser_status() || !this->client->getNick_status())
		return;
	if (this->args[0].empty() || this->args[0][0] != '#')
		return this->errorMsg("403", this->args[0], "No such channel");
	if (!this->server->getChannel_status(this->args[0])) //442 ERR_NOTONCHANNEL
		return this->errorMsg("442", this->args[0], "You're not on that channel");
	std::vector<Channel *>::iterator it = this->server->getChannel().begin();
	while ((*it)->getName() != this->args[0])
		it++;
	if (!(*it)->checkActive_users(this->client)) //442 ERR_NOTONCHANNEL
		return this->errorMsg("442", this->args[0], "You're not on that channel");
	if (!(*it)->checkOperators(client)) //482 ERR_CHANOPRIVSNEEDED
		return this->errorMsg("482", this->args[0], "You're not channel operator");
	(*it)->kickUser(this->client, args[1], this->last_param);
	if ((*it)->checkNo_users())
	{
		delete (*it);
		this->server->getChannel().erase(it);
	}
}

void Cmd::parse_privmsg( void )
{
	if (this->args.size() < 1) // 411 ERR_NORECIPIENT
		return this->errorMsg("411", "", "No recipient given (" + this->cmd + ")");
	if (!this->is_last_param) // 412 ERR_NOTEXTTOSEND
		return this->errorMsg("412", "", "No text to send");
	std::string dest;
	std::string dest_list = this->args[0];
	while (1){
		dest = dest_list.substr(0, dest_list.find_first_of(','));
		if (dest_list.find_first_of(',') == std::string::npos)
			dest_list.clear();
		else
			dest_list.erase(0, dest_list.find_first_of(',') + 1);
		this->privmsg(dest);
		if (dest_list.empty())
			break;
	}
}

void Cmd::privmsg( std::string dest )
{
	if (!this->client->getUser_status() || !this->client->getNick_status() || dest.empty())
		return;
	std::string msg = ":" + this->client->getNickname() + " PRIVMSG " + dest + " :" + this->last_param + "\n";
	if (dest[0] == '#')
	{
		std::vector<Channel *>::iterator it = this->server->getChannel().begin();
		std::vector<Channel *>::iterator ite = this->server->getChannel().end();
		while (it != ite && (*it)->getName() != dest)
			it++;
		if (it == ite) //403 ERR_NOSUCHCHANNEL
			return this->errorMsg("403", dest, "No such channel");
		if ((*it)->checkActive_users(this->client) == false) // 404 ERR_CANNOTSENDTOCHAN
			return this->errorMsg("404", dest, "Cannot send to channel");
		(*it)->printAllExceptClient(msg, this->client);
	}
	else
	{
		std::vector<Client *>::iterator it = this->server->getClient().begin();
		std::vector<Client *>::iterator ite = this->server->getClient().end();
		while (it != ite && (*it)->getNickname() != dest)
			it++;
		if (it == ite) //401 ERR_NOSUCHNICK
			return this->errorMsg("401", dest, "No such nick/channel");
		send((*it)->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
	}
}

void Cmd::notice( void )
{
	if (this->args.size() < 1) 
		return;
	if (!this->is_last_param) 
		return;
	if (!this->client->getUser_status() || !this->client->getNick_status() || this->args[0].empty())
		return;
	std::string msg = ":" + this->client->getNickname() + " NOTICE " + this->args[0] + " :" + this->last_param + "\n";
	if (this->args[0][0] == '#')
	{
		std::vector<Channel *>::iterator it = this->server->getChannel().begin();
		std::vector<Channel *>::iterator ite = this->server->getChannel().end();
		while (it != ite && (*it)->getName() != this->args[0])
			it++;
		if (it == ite) 
			return ;
		if ((*it)->checkActive_users(this->client) == false) 
			return ;
		(*it)->printAllExceptClient(msg, this->client);
	}
	else
	{
		std::vector<Client *>::iterator it = this->server->getClient().begin();
		std::vector<Client *>::iterator ite = this->server->getClient().end();
		while (it != ite && (*it)->getNickname() != this->args[0])
			it++;
		if (it == ite) 
			return ;
		std::cout << msg;
		send((*it)->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
	}
}

void Cmd::who( void ) 
{
	if (!this->client->getUser_status() || !this->client->getNick_status())
		return;
	std::vector<Client *>::iterator it_client = this->server->getClient().begin();
	std::vector<Client *>::iterator ite_client = this->server->getClient().end();
	while (it_client != ite_client)
	{
		if ((*it_client)->getUser_status() && (*it_client)->getNick_status())
		{
			bool is_matching = false;
			std::vector<Channel *>::iterator it_channel = this->server->getChannel().begin();
			std::vector<Channel *>::iterator ite_channel = this->server->getChannel().end();
			std::string channel_name = "";
			std::string channel_rights = "";
			while (it_channel != ite_channel)
			{
				if ((*it_channel)->checkActive_users(*it_client))
				{
					channel_name = (*it_channel)->getName();
					if (this->args.size() < 1 || channel_name == this->args[0])
					{
						is_matching = true;
						if ((*it_channel)->checkOperators(*it_client))
							channel_rights = " H";
						else
							channel_rights = "";
						if (channel_name == this->args[0])
							break;
					}
				}
				it_channel++;
			}
			if (channel_name == "")
				channel_name = "*";
			if (this->args.size() < 1 || (*it_client)->getHost() == this->args[0])
				is_matching = true;
			if (is_matching) // 352 RPL_WHOREPLY
			{
				std::stringstream ss;
				ss << this->server->getPort();

				this->replyMsg("352", channel_name + " ~" + (*it_client)->getUsername() + " " \
				+ (*it_client)->getHost() + " localhost/" + ss.str() + " " + \
				(*it_client)->getNickname() + channel_rights, "0 " + (*it_client)->getRealname());
			}
		}
		it_client++;
	}
	// 315 RPL_ENDOFWHO
	if (this->args.size() < 1)
		this->replyMsg("315", "*", "End of /Who list");
	else
		this->replyMsg("315", this->args[0], "End of /Who list");
}

void Cmd::cap( void )
{

}

void Cmd::pong( void )
{
	this->client->setIs_pong(true);
}

void Cmd::errorMsg(std::string code, std::string cmd, std::string what)
{
	std::string err;
	if (cmd != "")
		err = ":" + this->client->getServer_name() + " " + code + " " + this->client->getNickname() + " " + cmd + " :" + what + "\n";
	else
		err = ":" + this->client->getServer_name() + " " + code + " " + this->client->getNickname() + " :" + what + "\n";
	send(this->client->getFd(), err.c_str(), sizeof(const char) * err.length(), 0);
}

void Cmd::replyMsg(std::string code, std::string cmd, std::string what)
{
	std::string msg;
	if (cmd != "")
		msg = ":" + this->client->getServer_name() + " " + code + " " + this->client->getNickname() +  " " + cmd + " :" + what + "\n";
	else
		msg = ":" + this->client->getServer_name() + " " + code + " " + this->client->getNickname() + " :" + what + "\n";
	send(this->client->getFd(), msg.c_str(), sizeof(const char) * msg.length(), 0);
}

int Cmd::getIs_last_param( void )
{
	return this->is_last_param;
}