#include "CgiRequest.hpp"


/* Execption */
CgiRequest::cgiException::cgiException(string str) throw() : _str(str) 
{
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
CgiRequest::CgiRequest(void)
{
}

CgiRequest::CgiRequest(const unsigned short serverPort,
	HttpRequest const & request)
{
	_setEnv(0, string("AUTH_TYPE=")); //
	_setEnv(1, string("CONTENT_LENGTH=") + _toString(request._bodySize));
	_setEnv(2, string("CONTENT_TYPE=") /*+ request._fields.at("content_type")[0]*/); //
	_setEnv(3, string("GATEWAY_INTERFACE=CGI/1.1"));
	_setEnv(4, string("PATH_INFO=") + request._requiredFile);
	_setEnv(5, string("PATH_TRANSLATED=") + request._requiredFile);
	_setEnv(6, string("QUERY_STRING=") + request._queryPart); // apres ?
	_setEnv(7, string("REMOTE_ADDR=")); //
	_setEnv(8, string("REMOTE_IDENT=")); //
	_setEnv(9, string("REMOTE_USER=")); //
	_setEnv(10, string("REQUEST_METHOD=") + request._method); // ou POST
	_setEnv(11, string("REQUEST_URI=") + request._requiredFile);
	_setEnv(12, string("SCRIPT_NAME=") + request._requiredFile);
	_setEnv(13, string("SERVER_NAME=127.0.0.1"));
	_setEnv(14, string("SERVER_PORT=") + _toString(serverPort)); //
	_setEnv(15, string("SERVER_PROTOCOL=HTTP/1.1"));
	_setEnv(16, string("SERVER_SOFTWARE=Webserv/1.0"));
	_setEnv(17, string("REDIRECT_STATUS=200"));
	_env[18] = NULL;

	_setArg(0, request._requiredFile);
	_av[1] = NULL;
}

CgiRequest::~CgiRequest(void)
{
	for (int i = 0; _env[i] != NULL; ++i)
		delete _env[i];
	for (int i = 0; _av[i] != NULL; ++i)
		delete _av[i];
}

CgiRequest::CgiRequest(CgiRequest const & other)
{
	CgiRequest::_copy(other);
}

CgiRequest &
CgiRequest::operator=(CgiRequest const & other)
{
	if (this != &other)
		CgiRequest::_copy(other);
	return (*this);
}

/* Public method */
void 
CgiRequest::doRequest(void)
{
	int status;
	int p[2]; pipe(p);
	int child = fork();
	if (child == 0)
	{
		dup2(p[1], STDOUT);
		//if (execve("./cgitest/printenv", _av, _env) == -1)
		//if (execve("./testers/cgi_tester", _av, _env) == -1)
		if (execve("/Users/aalleman/.brew/bin/php-cgi", _av, _env) == -1)
		//if (execve("/usr/bin/php-cgi", _av, _env) == -1)
			exit(EXIT_FAILURE);
	}
	else
	{
		usleep(TIMEOUT);
		waitpid(child, &status, WNOHANG);
		if (WEXITSTATUS(status) == EXIT_FAILURE)
			throw(cgiException("execve fail"));
		kill(child, SIGKILL);
		s_buffer * buffer = NULL;
		do
		{
			buffer = new s_buffer(BUFFER_SIZE);
			buffer->occupiedSize = read(p[0], buffer->b, static_cast<size_t>(buffer->size));
			_answer.push(buffer);
			cout << buffer->occupiedSize << " / " << buffer->size << endl;
		} while (buffer->occupiedSize == buffer->size);
		if (buffer->occupiedSize == -1)
		{
			deleteQ(_answer);
			throw(cgiException("read fail"));
		}
	}
}

t_bufferQ const & 
CgiRequest::getAnswer(void) const
{
	return (_answer);
}

/* Private method */
void
CgiRequest::_copy(CgiRequest const & other)
{
	static_cast<void>(other);
}

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

template <class T>
string
CgiRequest::_toString(T number) const
{
	std::ostringstream ss;
	ss << number;
	return (ss.str());
}

/*void
CgiRequest::_analyseHeader(int fd)
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
		try { _request._parseHeaderLine(line);}
		catch (HttpRequest::parseException const & e)
		{ throw(cgiException(e.what()));}
	}
	if (lineSize < 0)
		throw(cgiException("recv error"));
	else if (headerSize > HEADER_MAX_SIZE)
		throw(cgiException("header too large"));
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
}*/