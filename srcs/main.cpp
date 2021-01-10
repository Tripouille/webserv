#include <cstring>
#include <netinet/in.h>
#include "TcpListener.hpp"
#include "ServerConfig.hpp"
#define PORT 8080
using std::cerr;
using std::endl;

int main(void)
{
	TcpListener webserv(INADDR_ANY, PORT);
	ServerConfig config;
	try
	{
		config.checkConfigFile();
		config.init();
		webserv.init();
		webserv.run();
	}
	catch (ServerConfig::configException const & e)
	{
		cerr << e.what() << endl;
	}
	catch (TcpListener::tcpException const & e)
	{
		cerr << e.what() << endl;
	}
	return (0);
}
