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

TcpListener::TcpListener(in_addr_t const & ipAddress, uint16_t port,
							ServerConfig & config, Host & host)
			: _ipAddress(ipAddress), _port(port), \
			  _backlog(BACKLOG), _clientNb(0), \
			  _config(config), _host(host)
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
TcpListener::_disconnectClient(SOCKET socket)
{
	cerr << "Killing socket " << socket << endl;
	FD_CLR(socket, &_activeFdSet);
	close(socket);
	_clientNb--;
}

void
TcpListener::_handleRequest(SOCKET socket) throw(tcpException)
{
	cout << endl << "Data arriving from socket " << socket << endl;
	HttpRequest request(_clientInfos[socket], _host, _port, _config);
	Answer answer(socket);

	try
	{
		try { request.analyze(); }
		catch (HttpRequest::parseException const & e)
		{ cerr << e.what() << endl; }
		catch (HttpRequest::closeOrderException const & e)
		{ _disconnectClient(socket); return ; }
		catch (HttpRequest::missingFileException const & e)
		{ return (_handleBadStatus(answer, request)); }

		// If it is a PUT request, update the files here and set status to 204 (or 201 if created)
		if (request._method == "PUT")
			_put(request);

		// Request is valid, no close order
		_answerToClient(socket, answer, request);
	}
	catch (Answer::sendException const & e)
	{
		cerr << e.what() << endl;
		_disconnectClient(socket);
	}
}

void
TcpListener::_put(HttpRequest & request) const
{
	struct stat fileInfos;

	if (stat(request._requiredFile.c_str(), &fileInfos) == 0)
	{
		if (S_ISDIR(fileInfos.st_mode) /*|| !S_ISREG(fileInfos.st_mode)*/)
		{
			request.setStatus(409, "Conflict");
			return ;
		}
		else
			request.setStatus(204, "No Content");
	}
	else
		request.setStatus(201, "Created");

	std::ofstream file(request._requiredFile.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (!file)
		request.setStatus(403, "Forbidden");
	else
	{
		file << request._body;
		file.close();
	}
}

void
TcpListener::_answerToClient(SOCKET socket, Answer & answer,
	HttpRequest & request)
	throw(tcpException, Answer::sendException)
{
	if (request._status.info == "Created" || request._status.info == "No Content")
	{
		answer.sendStatus(request._status);
		answer.sendEndOfHeader();
		return ;
	}
	else if (request._status.info != "OK"
	&& !(request._status.code == 404 && request._requiredFile.size()))
		return (_handleBadStatus(answer, request));

	string extension = request._requiredFile.substr(request._requiredFile.find_last_of('.') + 1, string::npos);
	bool requiredFileNeedCGI = (extension == "php");
	if (requiredFileNeedCGI)
		_doCgiRequest(CgiRequest(_port, request, _clientInfos[socket], _host, extension),
			request, answer);
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
	else if (request._status.code == 405)
	{
		answer._fields["Allow"] = "GET, HEAD"; // en ajouter ou en enlever selon la config
		answer.sendHeader();
	}
	answer.sendEndOfHeader();
	return (_disconnectClient(answer._client));
}

void
TcpListener::_doCgiRequest(CgiRequest cgiRequest, HttpRequest & request, Answer & answer)
{
	try { cgiRequest.doRequest(request, answer); }
	catch(CgiRequest::cgiException const & e)
	{
		request.setStatus(500, "Internal Server Error (CGI)");
		answer.sendStatus(request._status);
		answer.sendEndOfHeader();
		throw(Answer::sendException("error in cgi"));
	}
}