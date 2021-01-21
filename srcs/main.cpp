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

		/* Print debug ressource on class confi */
		// for (std::vector<Host>::iterator i = config.host.begin(); i != config.host.end(); ++i)
		// {
		// 	std::cout << "Server: " << i->serverName << std::endl << i->root << std::endl;
		// 	for (std::vector<uint16_t>::iterator port = i->port.begin(); port != i->port.end(); port++)
		// 		std::cout << *port << " - ";
		// 	std::cout << std::endl;
		// 	for (std::vector<string>::iterator index = i->index.begin(); index != i->index.end(); index++)
		// 		std::cout << *index << " - ";
		// 	std::cout << std::endl;
		// 	for (std::map<string, string>::iterator cgi = i->cgi.begin(); cgi != i->cgi.end(); cgi++)
		// 	{
		// 		std::cout << cgi->first << " : ";
		// 		std::cout << cgi->second << std::endl;
		// 	}
		// }

		/* Stop Serv */
		if (ac == 2)
		{
			serv.stopServ(av[1]);
			return (0);
		}
		/* Multi Serv */
		serv.initServs();
	}
	catch (ServerConfig::configException const & e)
	{
		cerr << e.what() << endl;
		exit(errno);
	}
	catch (TcpListener::tcpException const & e)
	{
		cerr << e.what() << endl;
		exit(errno);
	}
	catch (MultiServ::servException const & e)
	{
		cerr << e.what() << endl;
		exit(errno);
	}
	return (0);
}
