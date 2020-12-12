#include "TcpListener.hpp"
#include <cstdio>
#include <fstream>
#include <limits>

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

TcpListener::sendException::sendException(string str) throw()
						  : _str(str + " : " + strerror(errno))
{
}

TcpListener::sendException::~sendException(void) throw()
{
}

const char *
TcpListener::sendException::what(void) const throw()
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
	SOCKET client = accept(_socket, NULL, NULL);
	if (client < 0)
		throw tcpException("Accept failed");
	//std::cout << "Server : connect from host " << inet_ntoa(address.sin_addr)
	//		<< ", port " << ntohs(address.sin_port) << std::endl;
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
TcpListener::_handleRequest(SOCKET client)
{
	cout << endl << "Data arriving from socket " << client << endl;
	HttpRequest request(client);
	try { request.analyze(); }
	catch(HttpRequest::parseException const & e)
	{ cerr << e.what() << endl; }
	catch(HttpRequest::closeOrderException const & e)
	{ _disconnectClient(client); return ; }

	// Request is valid, no close order
	try { _answerToClient(client, request); }
	catch (sendException const & e)
	{
		cerr << e.what() << endl;
		_disconnectClient(client);
	}
}

void
TcpListener::_answerToClient(SOCKET client, HttpRequest & request)
									throw(sendException, tcpException)
{
	if (request._status.info != "OK" && request._status.code != 404)
	{
		_sendStatus(client, request.getStatus());
		return (_sendEndOfHeader(client));
	}
	string requiredFile = _getRequiredFile(request);
	struct stat fileInfos;
	if (stat(requiredFile.c_str(), &fileInfos) != 0)
	{
		request.setStatus(404, "Not Found");
		requiredFile = ROOT_DIRECTORY + string("/404.html");
	}
	_sendStatus(client, request.getStatus());
	bool requiredFileNeedCGI = false;
	if (requiredFileNeedCGI)
		return ;
	else if (stat(requiredFile.c_str(), &fileInfos) == 0)
		_sendFile(client, requiredFile.c_str(), fileInfos);
	else
	{
		cerr << "404.html not found" << endl;
		_sendEndOfHeader(client);
	}
}

string const
TcpListener::_getRequiredFile(HttpRequest const & request)
{
	if (request._target == "/")
		return (ROOT_DIRECTORY + string("/index.html"));
	else
		return (ROOT_DIRECTORY + request._target);
}

void
TcpListener::_sendToClient(SOCKET client, char const * msg, size_t size) const throw(sendException)
{
	if (send(client, msg, size, 0) == -1)
		throw(sendException("Could not send to client"));
}

void
TcpListener::_sendStatus(SOCKET client, HttpRequest::s_status const & status) const throw(sendException)
{
	std::ostringstream oss;
	oss << HTTP_VERSION << " " << status.code << " " << status.info << "\r\n";
	_sendToClient(client, oss.str().c_str(), oss.str().size());
}

void
TcpListener::_sendEndOfHeader(SOCKET client) const throw(sendException)
{
	_sendToClient(client, "\r\n", 2);
}

void
TcpListener::_sendFile(SOCKET client, char const * fileName,
						struct stat const & fileInfos)
						const throw(sendException, tcpException)
{
	std::ostringstream headerStream;

	_writeServerField(headerStream);
	_writeDateField(headerStream);
	t_bufferQ bufferQ = _getFile(fileName);
	_writeContentFields(headerStream, fileName, fileInfos, bufferQ);
	string header = headerStream.str();
	_sendToClient(client, header.c_str(), header.size());
	_sendEndOfHeader(client);
	_sendBody(client, bufferQ);
}

void
TcpListener::_sendBody(SOCKET client, t_bufferQ & bufferQ) const throw(sendException)
{
	while (!bufferQ.empty())
	{
		_sendToClient(client, bufferQ.front()->b, static_cast<size_t>(bufferQ.front()->occupiedSize));
		delete bufferQ.front();
		bufferQ.pop();
	}
}

void
TcpListener::_writeServerField(std::ostringstream & headerStream) const
{
	headerStream << "Server: webserv" << "\r\n";
}

void
TcpListener::_writeDateField(std::ostringstream & headerStream) const
{
	char date[50]; 
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	headerStream << "Date: " << date << "\r\n";
}

void
TcpListener::_writeContentFields(std::ostringstream & headerStream,
									char const * fileName,
									struct stat const & fileInfos,
									t_bufferQ const & bufferQ) const
{
	map<string, string> mimeTypes;
	mimeTypes["html"] = "text/html";
	mimeTypes["jpg"] = "image/jpeg";
	string extension = string(fileName).substr(string(fileName).find_last_of('.') + 1, string::npos);
	if (mimeTypes.count(extension))
		headerStream << "Content-Type: " << mimeTypes[extension] <<"\r\n";

	streamsize fileSize = static_cast<streamsize>(bufferQ.size() - 1)
							* bufferQ.back()->size + bufferQ.back()->occupiedSize;
	headerStream << "Content-Length: " << fileSize << "\r\n";

	time_t lastModified = fileInfos.st_mtime;
	struct tm tm = *gmtime(&lastModified);
	char date[50];
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	headerStream << "Last-Modified: " << date << "\r\n";
}

TcpListener::t_bufferQ
TcpListener::_getFile(char const * fileName) const
{
	t_bufferQ	bufferQ;
	s_buffer *	buffer;

	std::ifstream indexFile(fileName);
	if (!indexFile.is_open())
		throw(tcpException("Could not open file " + string(fileName)));
	do
	{
		buffer = new s_buffer(BUFFER_SIZE);
		indexFile.read(buffer->b, buffer->size); //throw
		bufferQ.push(buffer);
		buffer->occupiedSize = indexFile.gcount();
	} while (buffer->occupiedSize == buffer->size && !indexFile.eof());
	indexFile.close();

	return (bufferQ);
}