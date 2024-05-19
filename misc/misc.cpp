#include "misc.hpp"

int ft_error_msg( std::string msg )
{
	std::cerr << msg << std::endl;
	return (1);
}

void ft_handle_ctrlc(int signal){
	(void)signal;
	g_server->clearMemory();
	exit(1);
}