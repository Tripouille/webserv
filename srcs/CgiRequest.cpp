#include "CgiRequest.hpp"


/* Execption */
CgiRequest::cgiException::cgiException(string str, CgiRequest & cgiRequest)
throw() : _str(str)
{
	if (cgiRequest._inPipe[0] != UNSET_PIPE)
	{
		close(cgiRequest._inPipe[0]);
		close(cgiRequest._inPipe[1]);
	}
	if (cgiRequest._outPipe[0] != UNSET_PIPE)
	{
		close(cgiRequest._outPipe[0]);
		close(cgiRequest._outPipe[1]);
	}
}

CgiRequest::cgiException::~cgiException(void) throw()
{
}

const char *
CgiRequest::cgiException::what(void) const throw()
{
	return (_str.c_str());
}


/* CgiRequest */

CgiRequest::CgiRequest(const unsigned short serverPort,
			HttpRequest & request, Client const & client, string & cgi)
	: _cgi(cgi), _socket(client.s), _request(request)
{
	Client::authentication authentication;
	if (_request._requiredRealm.name.size())
		authentication = client.authentications.at(_request._requiredRealm.name);
	_setEnv(0, string("AUTH_TYPE=") + authentication.scheme);
	_setEnv(1, string("CONTENT_LENGTH=") + toStr(_request._bodySize));
	try { _setEnv(2, string("CONTENT_TYPE=") + _request._fields.at("content-type")[0]); }
	catch (std::out_of_range) {_setEnv(2, string("CONTENT_TYPE="));}
	_setEnv(3, string("GATEWAY_INTERFACE=CGI/1.1"));
	_setEnv(4, string("PATH_INFO=") + _request._requiredFile);
	_setEnv(5, string("PATH_TRANSLATED=") + _request._requiredFile);
	_setEnv(6, string("QUERY_STRING=") + _request._queryPart);
	_setEnv(7, string("REMOTE_ADDR=") + client.addr);
	_setEnv(8, string("REMOTE_IDENT=") + authentication.ident);
	_setEnv(9, string("REMOTE_USER=") + authentication.user);
	_setEnv(10, string("REQUEST_METHOD=") + _request._method);
	_setEnv(11, string("REQUEST_URI=") + _request._requiredFile);
	_setEnv(12, string("SCRIPT_NAME=") + _request._requiredFile);
	_setEnv(13, string("SERVER_NAME=127.0.0.1"));
	_setEnv(14, string("SERVER_PORT=") + toStr(serverPort));
	_setEnv(15, string("SERVER_PROTOCOL=HTTP/1.1"));
	_setEnv(16, string("SERVER_SOFTWARE=Webserv/1.0"));
	_setEnv(17, string("REDIRECT_STATUS=200"));
	_env[18] = NULL;

	_setArg(0, _request._requiredFile);
	_av[1] = NULL;

	_inPipe[0] = UNSET_PIPE;
	_outPipe[0] = UNSET_PIPE;
}

CgiRequest::~CgiRequest(void)
{
	for (int i = 0; _env[i] != NULL; ++i)
		delete[] _env[i];
	for (int i = 0; _av[i] != NULL; ++i)
		delete[] _av[i];
}

CgiRequest::CgiRequest(CgiRequest const & other)
	:  _cgi(other._cgi), _socket(other._socket), _request(other._request)
{
}

CgiRequest &
CgiRequest::operator=(CgiRequest const & other)
{
	static_cast<void>(other);
	return (*this);
}

/* Public method */
void
CgiRequest::doRequest(Answer & answer)
{
	int status;
	if (pipe(_inPipe) < 0 || pipe(_outPipe) < 0)
		throw(cgiException("pipe failed", *this));
	int child = fork();
	if (child < 0)
		throw(cgiException("fork failed", *this));
	if (child == 0)
	{
		dup2(_inPipe[0], STDIN);
		dup2(_outPipe[1], STDOUT);
		if (_request._bodySize)
			write(_inPipe[1], _request._body, _request._bodySize);
		char eof = -1; write(_inPipe[1], &eof, 1);
		if (execve(_cgi.c_str(), _av, _env) == -1)
			exit(EXIT_FAILURE);
	}
	usleep(TIMEOUT);
	waitpid(child, &status, WNOHANG);
	if (WEXITSTATUS(status) == EXIT_FAILURE)
		throw(cgiException("execve fail", *this));
	kill(child, SIGKILL);
	fcntl(_outPipe[0], F_SETFL, O_NONBLOCK);
	_analyzeHeader(_outPipe[0], answer);
	//answer._debugFields();
	s_buffer * buffer = NULL;
	do
	{
		buffer = new s_buffer(BUFF_SIZE);
		buffer->occupiedSize = read(_outPipe[0], buffer->b, static_cast<size_t>(buffer->size));
		if (buffer->occupiedSize >= 0)
			answer._body.push(buffer);
		//write(2, buffer->b, static_cast<size_t>(buffer->occupiedSize));
	} while (buffer->occupiedSize == buffer->size);
	if (buffer->occupiedSize == -1)
	{
		if (answer._body.empty())
			delete buffer;
		else
		{
			deleteQ(answer._body);
			throw(cgiException("read fail", *this));
		}
	}
	close(_inPipe[0]);
	close(_inPipe[1]);
	close(_outPipe[0]);
	close(_outPipe[1]);
}

/* Private method */
void
CgiRequest::_setEnv(int pos, string const & value)
{
	_env[pos] = new char[value.size() + 1];
	std::copy(value.begin(), value.end(), _env[pos]);
	_env[pos][value.size()] = 0;
}

void
CgiRequest::_setArg(int pos, string const & value)
{
	_av[pos] = new char[value.size() + 1];
	std::copy(value.begin(), value.end(), _av[pos]);
	_av[pos][value.size()] = 0;
}

void
CgiRequest::_analyzeHeader(int fd, Answer & answer)
{
	ssize_t			headerSize = 0;
	//+1 pour pouvoir lire un char supplémentaire et dépasser la limite
	char			line[HEADER_MAX_SIZE + 1];
	ssize_t			lineSize;

	while (headerSize <= HEADER_MAX_SIZE
	&& (lineSize = _getLine(fd, line, HEADER_MAX_SIZE)) > 0
	&& line[0])
	{
		headerSize += lineSize;
		_parseHeaderLine(line, answer);
	}
	if (lineSize < 0)
		throw(cgiException("recv error", *this));
	else if (headerSize > HEADER_MAX_SIZE)
		throw(cgiException("header too large", *this));
}

ssize_t
CgiRequest::_getLine(int fd, char * buffer, ssize_t limit) const
{
	ssize_t lineSize = 1;
	ssize_t	recvReturn = read(fd, buffer, 1);

	if (recvReturn <= 0)
		return (recvReturn);
	while (buffer[lineSize - 1] != '\n'
	&& buffer[lineSize - 1] != -1
	&& lineSize <= limit
	&& (recvReturn = read(fd, buffer + lineSize, 1)) == 1)
		++lineSize;

	if (recvReturn <= 0)
		return (recvReturn);
	else if (buffer[lineSize - 1] == -1)
		return (0);
	else if (lineSize > limit)
		return (lineSize);

	buffer[lineSize - 1] = 0;
	if (lineSize >= 2 && buffer[lineSize - 2] == '\r')
		buffer[lineSize - 2] = 0;
	return (lineSize);
}


void
CgiRequest::_parseHeaderLine(string line, Answer & answer) throw(cgiException)
{
	size_t colonPos = line.find(':', 0);
	if (colonPos == string::npos)
		throw(cgiException("no ':'", *this));
	string name = line.substr(0, colonPos);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	if (name.find(' ', 0) != string::npos)
		throw(cgiException("space before ':'", *this));
	string value = line.substr(line.find_first_not_of(' ', colonPos + 1), string::npos);
	if (name == "status")
		_extractStatus(value);
	else
		answer._fields[name] = value;
}

void
CgiRequest::_extractStatus(string & field) const
{
	size_t firstCharacter = field.find_first_not_of(' ');
	size_t lastCharacter = field.find_last_not_of(' ');
	field = field.substr(firstCharacter, lastCharacter);
	size_t spacePos = field.find(' ');
	int code = atoi(field.substr(0, spacePos).c_str());
	size_t statusPos = field.find_first_not_of(' ', spacePos);
	_request.setStatus(code, field.substr(statusPos));
}