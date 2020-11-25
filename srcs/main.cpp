#include <cstring>
//#include <sys/socket.h>
#include <netinet/in.h>
#include "TcpListener.hpp"
#define PORT 8080
using std::cerr;
using std::endl;

int main(void)
{
	sockaddr_in		address;
	size_t			addrlen = sizeof(address);

	memset(reinterpret_cast<char*>(&address), 0, addrlen);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	TcpListener webserv(reinterpret_cast<const char *>(&address), PORT);
	try
	{
		webserv.init();
	}
	catch (TcpListener::tcpException const & e)
	{
		cerr << e.what() << endl;
	}
	return (0);
}