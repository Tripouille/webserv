#include <cstring>
#include <netinet/in.h>

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
		config.init();
		MultiServ		servs(config, config.host);

		/* Stop Serv */
		if (ac == 2)
		{
			servs.stopServ(av[1]);
			return (0);
		}
		/* Multi Serv */
		servs.initServs();
	}
	catch (std::exception const & e)
	{
		cerr << e.what() << endl;
		exit(errno);
	}
	return (0);
}
