#include <cstring>
#include <netinet/in.h>
#include "TcpListener.hpp"
#include "ServerConfig.hpp"
#include "MultiServ.hpp"
#define PORT 8080
using std::cerr;
using std::endl;

int main(int ac, char *av[])
{
	ServerConfig	config;

	try
	{
		config.checkConfigFile();
		config.init();
		MultiServ		serv(config, config.host);

		/* Stop Serv */
		if (ac == 2)
		{
			serv.stopServ(av[1]);
			return (0);
		}
		/* Multi Serv */
		serv.initServs();
	}
	catch (std::exception const & e)
	{
		cerr << e.what() << endl;
		exit(errno);
	}
	return (0);
}
