#include <cstring>
#include <netinet/in.h>
#include "TcpListener.hpp"
#include "ServerConfig.hpp"
#define PORT 8080
using std::cerr;
using std::endl;

int main(int ac, char *av[])
{
	//TcpListener webserv(INADDR_ANY, PORT);
	ServerConfig config;

	if (ac == 2)
	{
		config.checkConfigFile();
		config.init();
		if (!(strcmp("stop", av[1])))
		{
			ifstream	file;
			string		arg;

			if (config.http.find("pid") != config.http.end())
			{
				file.open(config.http.at("pid").c_str());
				std::cout << "Extinction des serveurs !" << std::endl;
				while (getline(file, arg))
				{
					pid_t tmp;

					tmp = atoi(arg.c_str());
					kill(tmp, 15);
				}
			}
			file.close();
		}
		return (0);
	}
	try
	{
		config.checkConfigFile();
		config.init();

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

		/* Test multi Serv */
		ofstream			pids;
		int status;

		if (config.http.find("pid") != config.http.end())
			pids.open(config.http.at("pid").c_str());
		for (std::vector<Host>::iterator host = config.host.begin(); host != config.host.end(); host++)
		{
			for (std::vector<uint16_t>::iterator port = host->port.begin(); port != host->port.end(); port++)
			{
				pid_t pid;
				if ((pid = fork()) < 0)
				{
						std::cerr << "Error: fork failled" << std::endl;
						exit(10);
				}
				if (pid == 0)
				{
					pids << getpid() << std::endl;
					TcpListener webserv(INADDR_ANY, *port, config, *host);
					webserv.init();
					webserv.run();
				}
				else
				{
				}
			}
		}
		wait(&status);
		pids.close();

		//webserv.init();
		//webserv.run();
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
