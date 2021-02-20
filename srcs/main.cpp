#include <cstring>
#include <netinet/in.h>
#include "TcpListener.hpp"
#include "ServerConfig.hpp"
#include "MultiServ.hpp"
#include "Regex.hpp" // <===== include pour les regex.
#define PORT 8080
using std::cerr;
using std::endl;

int main(int ac, char *av[])
{
	/* Exemple Regex */
	try {
		Regex regex(".*\\.php$");
		std::cout << std::boolalpha << regex.match("lol.php") << " : " << regex.getLastMatch() << std::endl;
	}
	catch (std::invalid_argument const & e) {
		std::cerr << e.what() << std::endl;
	}
	/* Fin Exemple Regex */

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
