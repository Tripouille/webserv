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
	fd_set readfds, writefds;
	int socketCount;

	while (true)
	{
		timeval timeout = {60, 0}; // 60 seconds
		writefds = readfds = _activeFdSet;

		if ((socketCount = select(FD_SETSIZE, &readfds, &writefds, NULL, &timeout)) < 0)
		{
			close(_socket);
			throw tcpException("Select failed");
		}
	
		std::thread threads[WORKERS];
		for (int i = 0; i < WORKERS; ++i)
			threads[i] = std::thread(&TcpListener::_handleSocket, this, i, readfds, writefds);
		for (int i = 0; i < WORKERS; ++i)
			threads[i].join();
	}
}

/* Private */
void TcpListener::_handleSocket(size_t id, fd_set const & readfds, fd_set const & writefds) {
	for (SOCKET sock = id; sock < FD_SETSIZE; sock += WORKERS) {
		if (FD_ISSET(sock, &readfds)) {
			if (sock == _socket)
				_acceptNewClient();
			else if (FD_ISSET(sock, &writefds))
				_handleRequest(sock);
		}
	}
}

void
TcpListener::_acceptNewClient(void) throw(tcpException)
{
	cout << endl << "New connection to the server, ";
	struct sockaddr_in address; socklen_t address_len = sizeof(address);
	SOCKET s = accept(_socket, reinterpret_cast<sockaddr*>(&address), &address_len);
	if (s < 0)
		throw tcpException("Accept failed");
	_clientInfos[s] = Client(s, inet_ntoa(address.sin_addr));
	std::cout << "client address: " << _clientInfos[s].addr << endl;
	FD_SET(s, &_activeFdSet);
	struct timeval tv = {RCV_TIMEOUT, 0};
	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
	{
		close(s);
		throw tcpException("Setting socket option for client failed");
	}
	cout << ++_clientNb << " clients connected" << endl;
}

void
TcpListener::_disconnectClient(SOCKET socket)
{
	cout << "Killing socket " << socket << endl;
	FD_CLR(socket, &_activeFdSet);
	close(socket);
	_clientNb--;
}

void
TcpListener::_handleRequest(SOCKET socket) throw(tcpException)
{
	cout << endl << "Data arriving from socket " << socket << endl;
	HttpRequest request(_clientInfos[socket], _host, _config);
	Answer answer(socket, _host, _config);

	try
	{
		try { request.analyze(); }
		catch (HttpRequest::parseException const & e)
		{ cerr << e.what() << endl; }
		catch (HttpRequest::closeOrderException const & e)
		{ return (_disconnectClient(socket)); }
		catch (HttpRequest::directoryListingException const & e)
		{ _listDirectory(request, answer); return (_disconnectClient(socket));}

		if (request._status.code / 100 == 2
		&& (request._method == "PUT" || (request._method == "POST" && !request._fileFound))
		&& _getCgiPath(request._fileWithoutRoot).empty())
			_writeInFile(request);

		if (request._status.code / 100 != 2)
		{
			_setErrorFields(request, answer);
			if (!_setErrorPage(request))
				return (_handleNoErrorPage(answer, request));
		}
		_answerToClient(socket, answer, request);

		if (request._status.code / 100 != 2)
			_disconnectClient(socket);
	}
	catch (Answer::sendException const & e)
	{
		cerr << e.what() << endl;
		_disconnectClient(socket);
	}
}

void
TcpListener::_listDirectory(HttpRequest & request, Answer & answer) const
{
	string page;
	page += "<html><head>";
	page += "<style type=\"text/css\">table{font-size: 18px; border-collapse: collapse;} "
										"th {border-bottom: 1px black solid;} "
										"td {height: 26px; padding-right: 20px;}</style>";
	page += "</head><body>";
	page += "<h1>Index of " + request._fileWithoutRoot + "</h1>";
	page += "<table>";
	page += "<tr style=\"height: 40px;\"><th>Name</th><th>Last Modified</th><th>Size</th></tr>";
	DIR *dir;
	if ((dir = opendir(request._requiredFile.c_str())) != NULL)
	{
		struct stat fileInfos;
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
		{
			string fileName = ent->d_name;
			if (fileName == ".")
				continue ;
			stat(string(request._requiredFile + fileName).c_str(), &fileInfos);
			time_t lastModified = fileInfos.st_mtime;
			struct tm tm = *gmtime(&lastModified);
			char date[50];
			strftime(date, sizeof(date), "%d-%b-%Y %H:%M", &tm);
			page += "<tr>";
			if (fileName == "..")
				page += "<td><a href=\"" + request._fileWithoutRoot + fileName + "\">Parent Directory</a></td><td></td><td>-</td>";
			else
			{
				page += "<td><a href=\"" + request._fileWithoutRoot + fileName + "\">" + fileName + "</a></td>";
				page += "<td>" + string(date) + "</td>";
				page += string("<td>") + toStr(fileInfos.st_size) + string("</td>");
			}
			page += "</tr>";
		}
		closedir(dir);
	}
	page += "</table></body></html>";

	answer.sendStatus(request._status);
	answer.sendHeader();
	answer.sendEndOfHeader();
	if (request._method != "HEAD")
		answer._sendToClient(page.c_str(), page.size());
}

void
TcpListener::_writeInFile(HttpRequest & request) const
{
	struct stat fileInfos;

	if (stat(request._requiredFile.c_str(), &fileInfos) == 0)
	{
		if (S_ISDIR(fileInfos.st_mode))
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
		file.write(request._body, static_cast<streamsize>(request._bodySize));
		file.close();
	}
}

void
TcpListener::_writeInFile(HttpRequest & request, t_bufferQ body) const
{
	struct stat fileInfos;

	if (stat(request._requiredFile.c_str(), &fileInfos) == 0)
	{
		if (S_ISDIR(fileInfos.st_mode))
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
		while (!body.empty())
		{
			file.write(body.front()->b, body.front()->occupiedSize);
			body.pop();
		}
		file.close();
	}
}

void
TcpListener::_answerToClient(SOCKET socket, Answer & answer,
	HttpRequest & request) throw(tcpException, Answer::sendException)
{
	if (request._requiredFile.size())
	{
		string cgi = _getCgiPath(request._fileWithoutRoot);
		if (cgi.size())
		{
			_doCgiRequest(CgiRequest(_port, request, _clientInfos[socket], cgi), answer);
			if (request._method == "PUT" || (request._method == "POST" && !request._fileFound))
				_writeInFile(request, answer._body);
		}
		else if (request._method == "GET" || request._method == "HEAD")
		{
			try { answer.getFile(request._requiredFile); }
			catch (Answer::sendException const &) { throw(tcpException("File reading failed")); }
		}
	}
	answer.sendAnswer(request);
}

string const
TcpListener::_getCgiPath(string const & fileName) const
{
	string extension = fileName.substr(fileName.find_last_of('.') + 1);
	if (_host.cgi.count(extension))
		return (_host.cgi[extension]);
	return (string());
}

void
TcpListener::_doCgiRequest(CgiRequest cgiRequest, Answer & answer) const
{
	try { cgiRequest.doRequest(answer); }
	catch(CgiRequest::cgiException const & e)
	{
		cgiRequest._request.setStatus(500, "Internal Server Error (CGI)");
		answer.sendStatus(cgiRequest._request._status);
		answer.sendEndOfHeader();
		throw(Answer::sendException("error in cgi : " + string(e.what())));
	}
}

void
TcpListener::_setErrorFields(HttpRequest const & request, Answer & answer) const
{
	if (request._status.code == 401)
		answer._fields["WWW-Authenticate"] = string("Basic realm=\"")
											+ request._requiredRealm.name + string("\"");
	else if (request._status.code == 405)
	{
		vector<string> allowedMethods;
		string analyzedFile = request._fileWithoutRoot;
		map<string, vector<string> > location = request._getDeepestLocation("allowed_methods", analyzedFile);
		if (!location.empty())
			allowedMethods = location["allowed_methods"];
		vector<string>::const_iterator it = allowedMethods.begin();
		while (it != allowedMethods.end())
		{
			answer._fields["Allow"] += *it++;
			if (it != allowedMethods.end())
				answer._fields["Allow"] += ", ";
		}
	}
}

bool
TcpListener::_setErrorPage(HttpRequest & request) const
{
	struct stat fileInfos;
	string code = toStr(request._status.code);
	if (_host.errorPage.find(code) != _host.errorPage.end())
	{
		request._requiredFile = request._getPath(_host.errorPage[code]);
		if (stat(request._requiredFile.c_str(), &fileInfos) != 0 || !S_ISREG(fileInfos.st_mode))
			return (false);
		return (true);
	}
	return (false);
}

void
TcpListener::_handleNoErrorPage(Answer & answer, HttpRequest const & request)
{
	std::ostringstream ss; ss << request._status.code << " " << request._status.info;
	answer._fields["Content-Length"] = toStr(ss.str().size());

	answer.sendStatus(request._status);
	answer.sendHeader();
	answer.sendEndOfHeader();
	if (request._method != "HEAD")
		answer._sendToClient(ss.str().c_str(), ss.str().size());
	return (_disconnectClient(answer._client));
}