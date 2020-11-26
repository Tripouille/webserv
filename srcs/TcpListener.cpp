#include "TcpListener.hpp" 

/* tcpException */

TcpListener::tcpException::tcpException(string str) throw()
						  : _str(str + " : " + strerror(errno))
{
}

const char *
TcpListener::tcpException::what(void) const throw()
{
	return (_str.c_str());
}

/* Constructors and destructor */

TcpListener::TcpListener(in_addr_t const & ipAddress, uint16_t port)
			: _ipAddress(ipAddress), _port(port), _backlog(BACKLOG), _clientNb(0)
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

	if (listen(_socket, _backlog) < 0)
	{
		close(_socket);
		throw tcpException("Listen failed");
	}

	FD_ZERO(&_activeFdSet);
	FD_SET(_socket, &_activeFdSet);
}

void
TcpListener::_disconnectClient(SOCKET client)
{
	cerr << "Killing socket " << client << endl;
	FD_CLR(client, &_activeFdSet);
	close(client);
	_clientNb--;
}

void
TcpListener::run(void)
{
	fd_set setCopy;
	bool running = true;
	while (running)
	{
		setCopy = _activeFdSet;
		timeval timeout = {10, 0}; //sec, usec
		int socketCount = select(FD_SETSIZE, &setCopy, NULL, NULL, &timeout);
		if (socketCount < 0)
		{
			close(_socket);
			throw tcpException("Select failed");
		}

		// Service all the sockets with input pending
		for (SOCKET sock = 0; sock < FD_SETSIZE; ++sock)
		{
			if (FD_ISSET(sock, &setCopy))
			{
				if (sock == _socket)
					_acceptNewClient();
				else
					_receiveData(sock);
			}
		}
	}
}

void
TcpListener::_acceptNewClient(void)
{
	cout << endl << "New connection to the server" << endl;
	SOCKET client = accept(_socket, NULL, NULL);
	if (client < 0)
		throw tcpException("Accept failed");
	//std::cout << "Server : connect from host " << inet_ntoa(address.sin_addr)
	//		<< ", port " << ntohs(address.sin_port) << std::endl;
	FD_SET(client, &_activeFdSet);
	cout << ++_clientNb << " clients connected" << endl;

}

void
TcpListener::_receiveData(SOCKET client)
{
	cout << endl << "Data arriving from socket " << client << endl;
	char		buffer[CLIENT_MAX_BODY_SIZE + 2];
	ssize_t		nbytes;
	s_status	status = {200, "OK"};

	if ((nbytes = recv(client, buffer, CLIENT_MAX_BODY_SIZE + 1, 0)) < 0)
	{
		_disconnectClient(client);
		status.code = 500; status.info = "Internal Server Error (Cannot recv)";
		// A GERER : fonction pour envoyer erreur
	}
	else if (nbytes == CLIENT_MAX_BODY_SIZE + 1)
	{
		_disconnectClient(client);
		status.code = 413; status.info = "Entity Too Large";
		// A GERER : fonction pour envoyer erreur
	}
	else if (nbytes == 0) // end of file
		_disconnectClient(client);
	else
	{
		buffer[nbytes] = 0;
		//renverra structure allouée avec infos de la requête
		request req = _parseRequest(buffer, status);
		// appel de fonction pour répondre ou renvoyer une erreur
		//cout << "buffer = " << buffer << endl;
	}
}

		/*struct request
		{
			string 				method, filePath, httpVersion;
			map<string, string>	fields;
			string				body;
		};*/
TcpListener::request
TcpListener::_parseRequest(char * buffer, s_status & status) const
{
	std::istringstream iss(buffer);
	string line;
	getline(iss, line); // max 1024
	if (line.size() > REQUEST_LINE_MAX_SIZE)
	{
		status.code = 400; status.info = "Bad Request";
		cerr << "REQUEST_LINE_MAX_SIZE error to handle" << endl;
	}
	vector<string> startLine = _split(line, ' ');
	if (startLine.size() != 3)
	{
		status.code = 400; status.info = "Bad Request";

	}
	cerr << line << endl;
	return (request());
}

vector<string>
TcpListener::_split(string & s, char delim) const
{
	size_t			pos = 0;
	vector<string>	res;
	while ((pos = s.find(delim)) != string::npos && res.size() < 4)
	{
		res.push_back(s.substr(0, pos));
		s.erase(0, pos + 1);
	}
	res.push_back(s.substr(0, pos));
	return (res);
}

// verifier \r\n
void
TcpListener::_sendStatus(SOCKET client, s_status const & status)
{
	std::ostringstream oss;
	oss << HTTP_VERSION << " " << status.code << " " << status.info << "\r\n";
	send(client, oss.str().c_str(), oss.str().size(), 0);
}