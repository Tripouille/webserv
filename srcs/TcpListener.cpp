#include "TcpListener.hpp"
#include <cstdio>

/* Exceptions */

TcpListener::tcpException::tcpException(string str) throw()
						  : _str(str + " : " + strerror(errno))
{
}

TcpListener::tcpException::~tcpException(void) throw()
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
TcpListener::_acceptNewClient(void) throw(tcpException)
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
	HttpRequest request(client);
	try
	{
		request.analyze();
	}
	catch(const HttpRequest::parseException & e)
	{
		cerr << e.what() << endl;
	}
	











	/*char		buffer[CLIENT_MAX_BODY_SIZE];
	ssize_t		lineSize = _getLine(buffer, client);

	if (lineSize == 0) // end of file
	{
		_disconnectClient(client);
		return ;
	}
	else if (lineSize < 0)
		status.set(500, "Internal Server Error (Cannot recv)");
	//else if (lineSize == CLIENT_MAX_BODY_SIZE + 1)
		//status.set(413, "Entity Too Large");
	else
	{
		try
		{
			s_request req = _parseRequest(buffer, status);
		}
		catch (parseException const & e)
		{
			cerr << e.what() << endl;
		}
	}
	_sendStatus(client, status);
	send(client, "\r\n", 2, 0);
	if (status.info != "OK")
		_disconnectClient(client);
	else {//message
	}*/
}

TcpListener::s_request
TcpListener::_parseRequest(char * buffer, s_status & status) const throw(parseException)
{
	std::istringstream	iss(buffer);
	s_request			request;

	_parseRequestLine(iss, request, status);
	return (request);
}

void
TcpListener::_parseRequestLine(std::istringstream & iss, s_request & request,
								s_status & status) const throw(parseException)
{
	string				line;

	getline(iss, line);
	if (line.size() > REQUEST_LINE_MAX_SIZE)
	{
		status.set(400, status.info = "Bad Request");
		throw parseException("Bad Request : Request Line Too Long");
	}
	vector<string> requestLine = _split(line, ' ');
	if (requestLine.size() != 3)
	{
		status.set(400, status.info = "Bad Request");
		throw parseException("Bad Request : Invalid Request Line");
	}
	request.method = requestLine[0];
	request.target = requestLine[1];
	request.httpVersion = requestLine[2];
	_checkMethod(request.method, status);
	_checkTarget(request.target, status);
	_checkHttpVersion(request.httpVersion, status);
	cerr << "Request line : " << line << endl;
}

void
TcpListener::_checkMethod(string const & method, s_status & status) const throw(parseException)
{
	if (method != "GET" && method != "HEAD")
	{
		status.set(501, status.info = "Not Implemented");
		throw parseException("Not Implemented : Bad Method " + method);
	}
}

void
TcpListener::_checkTarget(string const & target, s_status & status) const throw(parseException)
{
	// 404 not found
	(void)target;
	(void)status;
}

void
TcpListener::_checkHttpVersion(string const & httpVersion, s_status & status) const throw(parseException)
{
	if (httpVersion != "HTTP/1.0" && httpVersion != "HTTP/1.1")
	{
		status.set(505, status.info = "HTTP Version Not Supported");
		throw parseException("HTTP Version Not Supported : " + httpVersion);
	}
}


/*
** This split only accepts one delimitor between each word.
** If there are delimitors before the string or several delimitors between words,
** it will return empty strings.
*/

vector<string>
TcpListener::_split(string s, char delim) const
{
					pos = 0;
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