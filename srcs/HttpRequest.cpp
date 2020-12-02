#include "HttpRequest.hpp"
#include <sys/socket.h>

/* Exceptions */

HttpRequest::parseException::parseException(HttpRequest const & request,
					string errorMsg, int code, string const & info) throw()
			: _str(info + " : " + errorMsg)
{
	const_cast<HttpRequest &>(request).setStatus(code, info);
}

HttpRequest::parseException::~parseException(void) throw()
{
}

const char *
HttpRequest::parseException::what(void) const throw()
{
	return (_str.c_str());
}


HttpRequest::closeOrderException::closeOrderException(void) throw()
{
}

HttpRequest::closeOrderException::~closeOrderException(void) throw()
{
}

const char *
HttpRequest::closeOrderException::what(void) const throw()
{
	return ("Client closed the connection");
}

/* HttpRequest */

HttpRequest::HttpRequest(SOCKET client)
			: _client(client)
{
	setStatus(200, "OK");
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

HttpRequest::s_status const &
HttpRequest::getStatus(void) const
{
	return (_status);
}

void
HttpRequest::setStatus(int c, string const & i)
{
	_status.code = c;
	_status.info = i;
}

void
HttpRequest::analyze(void) throw(parseException, closeOrderException)
{
	_analyseRequestLine();
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
HttpRequest::_analyseRequestLine(void) throw(parseException, closeOrderException)
{
	//+1 pour pouvoir lire un char supplémentaire et dépasser la limite
	char	buffer[REQUEST_LINE_MAX_SIZE + 1];
	ssize_t	lineSize;

	lineSize = _getLine(buffer, REQUEST_LINE_MAX_SIZE);
	if (lineSize < 0)
		throw(parseException(*this, "recv error", 500, "Internal Server Error"));
	else if (lineSize == 0)
		throw(closeOrderException());
	else if (lineSize > REQUEST_LINE_MAX_SIZE)
		throw(parseException(*this, "request line too long", 431, "Request Line Too Long"));
	vector<string> requestLine = _split(buffer, ' ');
	if (requestLine.size() != 3)
		throw parseException(*this, "invalid request line : " + string(buffer), 400, "Bad Request");
	_method = requestLine[0];
	_target = requestLine[1];
	_httpVersion = requestLine[2];
	_checkMethod();
	_checkTarget();
	_checkHttpVersion();
	cerr << "Request line : " << buffer << endl;
}

ssize_t
HttpRequest::_getLine(char * buffer, ssize_t limit) const throw(parseException)
{
	ssize_t lineSize = 1;
	ssize_t	recvReturn = recv(_client, buffer, 1, 0);

	if (recvReturn <= 0)
		return (recvReturn);
	while (buffer[lineSize - 1] != '\n'
	&& lineSize <= limit
	&& (recvReturn = recv(_client, buffer + lineSize, 1, 0)) == 1)
		++lineSize;
	if (recvReturn <= 0)
		return (recvReturn);
	else if (lineSize > limit)
		return (lineSize);
	--lineSize;
	buffer[lineSize] = 0;
	if (lineSize && buffer[lineSize - 1] == '\r')
		buffer[--lineSize] = 0;
	return (lineSize);
}

/*
** This split only accepts one delimitor between each word.
** If there are delimitors before the string or several delimitors between words,
** it will return empty strings.
*/

vector<string>
HttpRequest::_split(string s, char delim) const
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

void
HttpRequest::_checkMethod(void) const throw(parseException)
{
	if (_method != "GET" && _method != "HEAD")
		throw parseException(*this, "bad method : " + _method, 501, "Not Implemented");
}

void
HttpRequest::_checkTarget(void) const throw(parseException)
{
	if (_target.size() > URI_MAX_SIZE)
	{
		std::ostringstream oss; oss << _target.size();
		throw parseException(*this, oss.str(), 414, "URI Too Long");
	}
}

void
HttpRequest::_checkHttpVersion(void) const throw(parseException)
{
	if (_httpVersion != "HTTP/1.0" && _httpVersion != "HTTP/1.1")
		throw parseException(*this, "version [" + _httpVersion + "]", 505, "HTTP Version Not Supported");
}