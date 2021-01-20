#include "TcpListener.hpp"
#include <cstdio>
#include <fstream>
#include <limits>

/* Exception */

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

/* Member functions */
/* Public */
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
	int n = 1; struct timeval tv = {0, RCV_TIMEOUT};
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n)) < 0
	|| setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
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
TcpListener::run(void)
{
	fd_set setCopy;
	bool running = true;
	while (running)
	{
		setCopy = _activeFdSet;
		timeval timeout = {60, 0}; // 60 seconds
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
					_handleRequest(sock);
			}
		}
	}
}

/* Private */
void
TcpListener::_acceptNewClient(void) throw(tcpException)
{
	cout << endl << "New connection to the server" << endl;
	struct sockaddr_in address; socklen_t address_len = sizeof(address);
	SOCKET client = accept(_socket, reinterpret_cast<sockaddr*>(&address), &address_len);
	if (client < 0)
		throw tcpException("Accept failed");
	_clientInfos[client] = Client(client, inet_ntoa(address.sin_addr));
	std::cout << "client address: " << _clientInfos[client].addr << endl;
	FD_SET(client, &_activeFdSet);
	cout << ++_clientNb << " clients connected" << endl;
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
TcpListener::_handleRequest(SOCKET client) throw(tcpException)
{
	cout << endl << "Data arriving from socket " << client << endl;
	HttpRequest request(_clientInfos[client]);
	try { request.analyze(); }
	catch(HttpRequest::parseException const & e)
	{ cerr << e.what() << endl; }
	catch(HttpRequest::closeOrderException const & e)
	{ _disconnectClient(client); return ; }

	// Request is valid, no close order
	try { _answerToClient(client, request); }
	catch (Answer::sendException const & e)
	{
		cerr << e.what() << endl;
		_disconnectClient(client);
	}
}

void
TcpListener::_answerToClient(SOCKET client, HttpRequest & request)
	throw(tcpException, Answer::sendException)
{
	Answer answer(client);

	if (request._status.info != "OK"
	&& !(request._status.code == 404 && request._requiredFile.size()))
		return (_handleBadStatus(answer, request));

	string extension = request._requiredFile.substr(request._requiredFile.find_last_of('.') + 1, string::npos);
	bool requiredFileNeedCGI = (extension == "php");
	if (requiredFileNeedCGI)
	{
		CgiRequest cgiRequest(_port, request, _clientInfos[client]);
		try { cgiRequest.doRequest(request, answer); }
		catch(std::exception const & e)
		{
			cerr << e.what() << endl;
			request.setStatus(500, "Internal Server Error (CGI)");
			answer.sendStatus(request._status);
			answer.sendEndOfHeader();
			return (_disconnectClient(client));
		}
	}
	else
	{
		try { answer.getFile(request._requiredFile); }
		catch (Answer::sendException const &) { throw(tcpException("File reading failed")); }
	}
	answer.sendStatus(request._status);
	answer.sendAnswer(request);
}

void
TcpListener::_handleBadStatus(Answer & answer, HttpRequest const & request)
	throw(Answer::sendException)
{
	answer.sendStatus(request._status);
	if (request._status.code == 401)
	{
		answer._fields["WWW-Authenticate"] = string("Basic realm=\"")
											+ request._requiredRealm.name + string("\"");
		answer.sendHeader();
	}
	answer.sendEndOfHeader();
	return (_disconnectClient(answer._client));
}
