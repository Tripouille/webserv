#include "TcpListener.hpp" 

/* Constructors and destructor */

TcpListener::TcpListener(in_addr_t const & ipAddress, uint16_t port)
			: _ipAddress(ipAddress), _port(port), _backlog(BACKLOG)
{
}

TcpListener::~TcpListener()
{
	close(_socket);
}


/* Operators */


/* Getters and setters */


/* Member functions */

void
TcpListener::init(void)
{
	sockaddr_in		address;
	size_t			addrlen = sizeof(address);

	memset(reinterpret_cast<char*>(&address), 0, addrlen);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(_ipAddress);
	address.sin_port = htons(_port);

	if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw tcpException("Socket creation failed");

	// Flag SO_REUSEADDR pour éviter l'erreur "bind failed: Address already in use"
	int n = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n)) < 0)
	{
		close(_socket);
		throw tcpException("Setting socket option failed");
	}

	if (bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
	{
		close(_socket);
		throw tcpException("Bind failed");
	}
}

void
TcpListener::_killSocket(SOCKET sock)
{
	cerr << "Killing socket " << sock << endl;
	close(sock);
	FD_CLR(sock, &_activeFdSet);
}

void
TcpListener::run(void)
{
	if (listen(_socket, _backlog) < 0)
	{
		close(_socket);
		throw tcpException("Listen failed");
	}

	FD_ZERO(&_activeFdSet);
	FD_SET(_socket, &_activeFdSet); //places _socket in the fd_set activeFdSet

	fd_set setCopy;
	int clientNb = 0;
	bool running = true;
	while (running)
	{
		setCopy = _activeFdSet;
		// int select(int ndffs, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval* timeout)
		timeval timeout = {10, 0}; // 10 secondes
		int socketCount = select(FD_SETSIZE, &setCopy, NULL, NULL, &timeout);
		if (socketCount < 0)
		{
			close(_socket);
			throw("Select failed");
		}

		// Service all the sockets with input pending
		for (SOCKET sock = 0; sock < FD_SETSIZE; ++sock)
		{
			if (FD_ISSET(sock, &setCopy))
			{
				if (sock == _socket)
				{
					// Connection request on original socket
					cout << endl << "Connection request on original socket" << endl;
					SOCKET client = accept(_socket, NULL, NULL);
					if (client < 0)
						throw("Accept failed");
					//std::cout << "Server : connect from host " << inet_ntoa(address.sin_addr)
					//		<< ", port " << ntohs(address.sin_port) << std::endl;
					FD_SET(client, &_activeFdSet);
					clientNb++;
					cout << clientNb << " clients connected" << endl;
				}
				else
				{
					// Data arriving on an already-connected socket
					cout << endl << "Data arriving on an already connected socket" << endl;
					char buffer[MAXMSG + 1];
					ssize_t nbytes;

					if ((nbytes = recv(sock, buffer, MAXMSG, 0)) < 0)
					{
						_killSocket(sock);
						clientNb--;
						cerr << "Error in recv" << endl; // A GERER
					}
					else if (nbytes == 0) // end of file
					{
						_killSocket(sock);
						clientNb--;
						std::cerr << "nbytes = 0" << std::endl;
					}
					else
					{
						std::cerr << "nbytes = " << nbytes << std::endl;
						buffer[nbytes] = 0;
						//read_data_and_answer(buffer, i);
						cout << "buffer = " << buffer << endl;
					}
				}
			}
		}
	}
}