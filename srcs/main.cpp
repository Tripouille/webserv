#include <cstring>
#include <netinet/in.h>
#include "TcpListener.hpp"
#define PORT 8080
using std::cerr;
using std::endl;

int main(void)
{

	TcpListener webserv(INADDR_ANY, PORT);
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