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
	_clientInfos[client].s = client;
	_clientInfos[client].addr = inet_ntoa(address.sin_addr);
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

	//Debug print fields
	map<string, vector<string> >::iterator it = request._fields.begin();
	map<string, vector<string> >::iterator ite = request._fields.end();
	for (; it != ite; ++it)
	{
		cout << "[" << it->first << "] = ";
		vector<string>::iterator vit = it->second.begin();
		vector<string>::iterator vite = it->second.end();
		for (; vit != vite; ++vit)
			cout << *vit << " ";
		cout << endl;
	}
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
	throw(tcpException)
{
	Answer answer(client);

	//TEST AUTH
	static int isAuth = 1;
	if (isAuth++ < 3)
	{
		string msg = "HTTP/1.1 401 Unauthorized\r\n";
		answer._sendToClient(msg.c_str(), msg.size());
		msg = "WWW-Authenticate: Basic realm=\"Access to the staging site\"\r\n";
		answer._sendToClient(msg.c_str(), msg.size());
		answer.sendEndOfHeader();
		return (_disconnectClient(client));
	}
	//TEST AUTH END

	if (request._status.info != "OK"
	&& !(request._status.code == 404 && !request._requiredFile.empty()))
	{
		answer.sendStatus(request._status);
		answer.sendEndOfHeader();
		return (_disconnectClient(client));
	}
	string extension = request._requiredFile.substr(request._requiredFile.find_last_of('.') + 1, string::npos);
	bool requiredFileNeedCGI = (extension == "php");
	if (requiredFileNeedCGI)
	{
		CgiRequest cgiRequest(_port, request, _clientInfos[client]);
		try { cgiRequest.doRequest(answer); }
		catch(std::exception const & e)
		{
			request.setStatus(500, "Internal Server Error (CGI)");
			answer.sendStatus(request._status);
			answer.sendEndOfHeader();
			return (_disconnectClient(client));
		}
		//answer.setBody(cgiRequest.getAnswer());
		cout << "first buffer cgiRequest : " << endl;
		write(1, answer._body.front()->b, (size_t)answer._body.front()->occupiedSize);
		write(1, "\n", 1);
	}
	else
	{
		try { answer.getFile(request._requiredFile); }
		catch (Answer::sendException const &) { throw(tcpException("File reading failed")); }
	}
	answer.sendStatus(request._status);
	answer.sendAnswer(request._requiredFile);
}