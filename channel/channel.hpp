#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "../ircserv.hpp"

class Client;
class Channel {

	public:
		Channel(std::string chan, std::string key);
		~Channel(void);

		std::string getName();
		std::string getKey();
		std::string getTopic();
		int			getUserLimit();
		bool		getKey_set();
		bool		getUser_limit_set();
		bool		getWhitelist_set();
		bool		getTopic_restrict();

		int			getUserNumber();

		void		addUser(Client *client);
		void		addOperator(Client *client);
		void		addOperator(std::string client, Client *c);
		void		removeOperator(std::string client, Client *c);
		void		addWhitelist(Client *client);
		void		removeUser(Client *client);
		void		kickUser(Client *client, std::string to_kick, std::string reason);

		bool		checkOperators(Client *client);
		bool		checkWhitelist(Client *client);
		bool		checkActive_users(Client *client);
		bool		checkActive_users(std::string nick);
		bool		checkUser_limit();
		bool		checkNo_users();

		
		void		print(std::string msg, Client *client);
		void		print(std::string msg, std::string client);
		void		printTopic(Client *client);
		void		printAll(std::string msg);
		void		printAllExceptClient(std::string msg, Client *client);
		void		printModes(Client *client);
		void		setTopic(Client *client, std::string topic);
		void		setTopic_restrict(bool b, Client *client);
		void		setWhitelist(bool b, Client *client);
		void		setKey(bool b, std::string key, Client *client);
		void		setUser_limit(bool b, std::string lim, Client *client);

		void		clearMemory();

		std::vector<Client *> &getActive_users();

	private:
		Channel( Channel const & src );
		Channel & operator=( Channel const & rhs );

		std::string name;
		std::string key;
		std::string topic;

		int			user_limit;

		bool		key_set;
		bool		user_limit_set;
		bool		whitelist_set;

		bool		topic_restrict;

		std::vector<Client *> active_users;
		std::vector<Client *> operators;
		std::vector<Client *> whitelist;
};

#endif