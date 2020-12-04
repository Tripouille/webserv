#include "HttpRequest.hpp"
#include <sys/socket.h>

/* Exceptions */

HttpRequest::parseException::parseException(HttpRequest const & request,
				 int code, string const & info, string errorMsg) throw()
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
	ssize_t headerSize = 0;
	
	_analyseRequestLine(headerSize);
	_analyseHeader(headerSize);
	_analyseBody();
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
	memcpy(_body, other._body, CLIENT_MAX_BODY_SIZE + 1);
	_status = other._status;
}

void
HttpRequest::_analyseRequestLine(ssize_t & headerSize) throw(parseException, closeOrderException)
{
	//+1 pour pouvoir lire un char supplémentaire et dépasser la limite
	char			buffer[REQUEST_LINE_MAX_SIZE + 1];
	vector<string>	requestLine;

	headerSize = _getLine(buffer, REQUEST_LINE_MAX_SIZE);
	if (headerSize < 0)
		throw(parseException(*this, 500, "Internal Server Error", "recv error"));
	else if (headerSize == 0)
		throw(closeOrderException());
	else if (headerSize > REQUEST_LINE_MAX_SIZE)
		throw(parseException(*this, 431, "Request Line Too Long", "request line too long"));

	requestLine = _splitRequestLine(buffer);
	if (requestLine.size() != 3)
		throw parseException(*this, 400, "Bad Request", "invalid request line : " + string(buffer));
	_fillAndCheckRequestLine(requestLine);

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

	buffer[lineSize - 1] = 0;
	if (lineSize >= 2 && buffer[lineSize - 2] == '\r')
		buffer[lineSize - 2] = 0;
	return (lineSize);
}

/*
** This split only accepts one delimitor between each word.
** If there are delimitors before the string or several delimitors between words,
** it will return empty strings.
*/

vector<string>
HttpRequest::_splitRequestLine(string s) const
{
	size_t			pos = 0;
	vector<string>	res;

	while ((pos = s.find(' ')) != string::npos && res.size() < 4)
	{
		res.push_back(s.substr(0, pos));
		s.erase(0, pos + 1);
	}
	res.push_back(s.substr(0, pos));
	return (res);
}

void
HttpRequest::_fillAndCheckRequestLine(vector<string> const & requestLine) throw(parseException)
{
	_method = requestLine[0];
	_target = requestLine[1];
	_httpVersion = requestLine[2];
	_checkMethod();
	_checkTarget();
	_checkHttpVersion();
}

void
HttpRequest::_checkMethod(void) const throw(parseException)
{
	if (_method != "GET" && _method != "HEAD")
		throw parseException(*this, 501, "Not Implemented", "bad method : " + _method);
}

void
HttpRequest::_checkTarget(void) const throw(parseException)
{
	if (_target.size() > URI_MAX_SIZE)
	{
		std::ostringstream oss; oss << _target.size();
		throw parseException(*this, 414, "URI Too Long", oss.str());
	}
	struct stat fileInfos;
	if (stat(_target.c_str(), &fileInfos) != 0)
		throw parseException(*this, 404, "Not Found", _target);
}

void
HttpRequest::_checkHttpVersion(void) const throw(parseException)
{
	if (_httpVersion != "HTTP/1.0" && _httpVersion != "HTTP/1.1")
		throw parseException(*this, 505, "HTTP Version Not Supported", "version [" + _httpVersion + "]");
}

void
HttpRequest::_analyseHeader(ssize_t & headerSize) throw(parseException)
{
	//+1 pour pouvoir lire un char supplémentaire et dépasser la limite
	char			line[HEADER_MAX_SIZE + 1];
	ssize_t			lineSize;

	while (headerSize <= HEADER_MAX_SIZE
	&& (lineSize = _getLine(line, HEADER_MAX_SIZE)) > 0
	&& line[0])
	{
		headerSize += lineSize;
		//cerr << "line = " << line << endl;
		_parseHeaderLine(line);
	}
	if (lineSize < 0)
		throw(parseException(*this, 500, "Internal Server Error", "recv error"));
	else if (headerSize > HEADER_MAX_SIZE)
		throw(parseException(*this, 431, "Request Header Fields Too Large", "header too large"));
}

void
HttpRequest::_parseHeaderLine(string line) throw(parseException)
{
	size_t colonPos = line.find(':', 0);
	if (colonPos == string::npos)
		throw(parseException(*this, 400, "Bad Request", "no :"));
	string name = line.substr(0, colonPos);
	if (name.find(' ', 0) != string::npos)
		throw(parseException(*this, 400, "Bad Request", "space before :"));
	string value = line.substr(colonPos + 1, string::npos);
	_splitHeaderField(value, _fields[name]);
	/*cerr << "name = " << name << ", value = " << value << endl;
	for (size_t i = 0; i < _fields[name].size(); ++i)
		cerr << _fields[name][i] << "|||";
	cerr << endl;*/
}

void
HttpRequest::_splitHeaderField(string s, vector<string> & fieldValue) const
{
	size_t			pos = 0;
	string			value;

	while ((pos = s.find(',')) != string::npos)
	{
		value = s.substr(0, pos);
		value.erase(0, value.find_first_not_of(" "));
		value.erase(value.find_last_not_of(" ") + 1);
		fieldValue.push_back(value);
		s.erase(0, pos + 1);
	}
	value = s.substr(0, pos);
	value.erase(0, value.find_first_not_of(" "));
	value.erase(value.find_last_not_of(" ") + 1);
	fieldValue.push_back(value);
}

// Tous les tests ont l'air bon, sauf que si on saute une ligne dans le body, ça arrête le recv ??
// Petit doute avec le close connection : les octets de la prochaine requête sont perdus ; et je suis pas sûre que le client attende la réponse avant d'envoyer la prochaine requête (voir chrome et sa requête pour favicon)

void
HttpRequest::_analyseBody(void)
{
	_body[0] = 0;
	unsigned int contentLength = _getContentLength();
	if (contentLength == 0)
		return ;
	cerr << "contentLength = " << contentLength << endl;
	ssize_t recvRet = recv(_client, _body, contentLength, 0);
	if (recvRet < 0)
		throw(parseException(*this, 500, "Internal Server Error", "recv error"));
	else if (recvRet < contentLength)
		throw(parseException(*this, 500, "Internal Server Error", "body smaller than given content length"));
	_body[contentLength] = 0;
	cerr << "Body : " << _body << endl;
}

unsigned int
HttpRequest::_getContentLength(void)
{
	if (_fields[string("Content-Length")].size() == 0)
		return (0);
	else if (_fields["Content-Length"].size() > 1)
		throw(parseException(*this, 400, "Bad Request", "Mutiple Content-Length fields"));
	else if (_fields["Content-Length"][0].size() > 9)
		throw(parseException(*this, 400, "Bad Request", "Content-Length field is too long"));
	else if (_fields["Content-Length"][0].find_first_not_of("0123456789") != string::npos)
		throw(parseException(*this, 400, "Bad Request", "Content-Length is not a number"));
	unsigned int contentLength;
	std::istringstream(_fields["Content-Length"][0]) >> contentLength;
	if (contentLength > CLIENT_MAX_BODY_SIZE)
		throw(parseException(*this, 413, "Payload Too Large", "Content-Length too high"));
	return (contentLength);
}
