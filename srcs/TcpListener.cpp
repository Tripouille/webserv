#include "TcpListener.hpp" 

/* Constructors and destructor */

TcpListener::TcpListener(const char * ipAddress, int port)
			: _address(ipAddress), _port(port), _backlog(BACKLOG)
{
}

TcpListener::~TcpListener()
{
}


/* Operators */


/* Getters and setters */


/* Member functions */

void
TcpListener::init(void)
{
	if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw tcpException("Socket creation failed");

	// Flag SO_REUSEADDR pour éviter l'erreur "bind failed: Address already in use"
	int n = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n)) < 0)
	{
		close(_socket);
		throw tcpException("Setting socket option failed");
	}

	if (bind(_socket, reinterpret_cast<sockaddr*>(&_address), sizeof(_address)) < 0)
	{
		close(_socket);
		throw tcpException("Bind failed");
	}

	if (listen(_socket, _backlog) < 0)
	{
		close(_socket);
		throw tcpException("Listen failed");
	}
}
