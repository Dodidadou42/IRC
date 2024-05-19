#include "ircserv.hpp"

Server *g_server;

int main(int argc, char **argv)
{
	signal(SIGINT, ft_handle_ctrlc);
	if (argc != 3)
		return(ft_error_msg("Error!\n./ircserv <port> <password>"));
	else
	{
		
		int port;
		std::istringstream iss(argv[1]);

		if (!(iss >> port))
			return(ft_error_msg("Error!\nport value"));
        
		std::string password = argv[2];

		Server *server;

		try
		{
			server = new Server(port, password);
			
			if (server == NULL)
				return(ft_error_msg("Error!\nallocation server"));
		}
		catch(std::exception & e)
		{
			return(1);
		}

		g_server = server;
		int ret = 0;

		while (ret == 0)
			ret = server->call_poll();
		delete server;
    }
}