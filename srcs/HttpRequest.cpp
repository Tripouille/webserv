#include "HttpRequest.hpp"
#include <sys/socket.h>

/* Exception */

HttpRequest::parseException::parseException(string str) throw() : _str(str)
{
}

HttpRequest::parseException::~parseException(void) throw()
{
}

const char *
HttpRequest::parseException::what(void) const throw()
{
	return (_str.c_str());
}

/* HttpRequest */

HttpRequest::HttpRequest(SOCKET client)
			: _client(client)
{
	_setStatus(200, "OK");
}

HttpRequest::~HttpRequest(void)
{
}

HttpRequest::HttpRequest(HttpRequest const & other)
{
	HttpRequest::_copy(other);
}

HttpRequest &
HttpRequest::operator=(HttpRequest const & other)
{
	if (this != &other)
		HttpRequest::_copy(other);
	return (*this);
}

/* Public */

void
HttpRequest::analyze(void) throw(parseException)
{
}

/* Private */

void
HttpRequest::_copy(HttpRequest const & other)
{
	_client = other._client;
	_method = other._method;
	_target = other._target;
	_httpVersion = other._httpVersion;
	_fields = other._fields;
	_body = other._body;
	_status = other._status;
}

void
HttpRequest::_setStatus(int c, string const & i)
{
	_status.code = c;
	_status.info = i;
}

ssize_t
HttpRequest::_getLine(char * buffer, size_t limit) const
{
	ssize_t lineSize = 1;
	int		recvReturn = recv(_client, buffer, 1, 0);

	if (recvReturn <= 0)
		return (recvReturn);
	while (buffer[lineSize - 1] != '\n'
	&& lineSize < limit
	&& (recvReturn = recv(_client, buffer + lineSize, 1, 0)) == 1)
		++lineSize;
	if (recvReturn <= 0)
		return (recvReturn);
	buffer[lineSize - 1] = 0;
	return (lineSize);
}