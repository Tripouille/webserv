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

HttpRequest::HttpRequest(Client & client, Host& host, ServerConfig & config)
			: _client(client), _host(host), _config(config)
{
	setStatus(200, "OK");
	_bodySize = 0;
}

HttpRequest::~HttpRequest(void)
{
}

HttpRequest::HttpRequest(HttpRequest const & other)
		: _client(other._client), _host(other._host), _config(other._config)
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
	//_debugFields();
	_setRequiredFile();
	if (!_methodIsAuthorized())
		throw(parseException(*this, 405, "Method Not Allowed", "from config"));
	_setRequiredRealm();
	if (_requiredRealm.name.size())
		_setClientInfos();
	if (!_isAuthorized())
		throw(parseException(*this, 401, "Unauthorized", "wrong credentials"));
	_analyseBody();
}

/* Private */

void
HttpRequest::_copy(HttpRequest const & other)
{
	_method = other._method;
	_target = other._target;
	_httpVersion = other._httpVersion;
	_fields = other._fields;
	memcpy(_body, other._body, CLIENT_MAX_BODY_SIZE + 1);
	_status = other._status;
}

/*
** The buffer is one character longer to be able to read one more
** and detect the limit was passed.
*/

void
HttpRequest::_analyseRequestLine(ssize_t & headerSize) throw(parseException, closeOrderException)
{
	char			buffer[REQUEST_LINE_MAX_SIZE + 1];
	vector<string>	requestLine;

	for (int i = 0; i <= atoi(_config.http.at("max_empty_line_before_request").c_str())
	&& (((headerSize = _getLine(buffer, REQUEST_LINE_MAX_SIZE)) == 2 && buffer[0] == 0)
	|| headerSize == 1); ++i)
		;
	if ((headerSize == 2 && buffer[0] == 0)
	|| headerSize == 1)
		throw(parseException(*this, 400, "Bad Request", "Too many empty lines before request"));
	if (headerSize < 0)
		throw(parseException(*this, 500, "Internal Server Error", "recv error (may have timeout)"));
	else if (headerSize == 0)
		throw(closeOrderException());
	else if (headerSize > REQUEST_LINE_MAX_SIZE)
		throw(parseException(*this, 431, "Request Line Too Long", "request line too long"));

	requestLine = _splitRequestLine(buffer);
	//std::ostringstream debug; debug << (int)*buffer;
	if (requestLine.size() != 3)
		throw parseException(*this, 400, "Bad Request", "invalid request line : "
		+ string(buffer)); //+ " / " + debug.str());
	_fillAndCheckRequestLine(requestLine);

	cerr << "Request line : " << buffer << endl;
}

ssize_t
HttpRequest::_getLine(char * buffer, ssize_t limit) const throw(parseException)
{
	ssize_t lineSize = 1;
	ssize_t	recvReturn = recv(_client.s, buffer, 1, 0);

	if (recvReturn <= 0)
		return (recvReturn);
	while (buffer[lineSize - 1] != '\n'
	&& buffer[lineSize - 1] != -1
	&& lineSize <= limit
	&& (recvReturn = recv(_client.s, buffer + lineSize, 1, 0)) == 1)
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
	if (_method != "GET" && _method != "HEAD" && _method != "POST" && _method != "PUT")
		throw parseException(*this, 501, "Not Implemented", "bad method : " + _method);
}

void
HttpRequest::_checkTarget(void) const throw(parseException)
{
	if (_target.size() > static_cast<unsigned int>(atoi(_config.http.at("uri_max_size").c_str())))
	{
		std::ostringstream oss; oss << _target.size();
		throw parseException(*this, 414, "URI Too Long", oss.str());
	}
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
		_parseHeaderLine(line);
	}
	if (lineSize < 0)
		throw(parseException(*this, 500, "Internal Server Error", "recv error"));
	else if (headerSize > HEADER_MAX_SIZE)
		throw(parseException(*this, 431, "Request Header Fields Too Large", "header too large"));
	_checkHeader();
}

void
HttpRequest::_parseHeaderLine(string line) throw(parseException)
{
	size_t colonPos = line.find(':', 0);
	if (colonPos == string::npos)
		throw(parseException(*this, 400, "Bad Request", "no ':'"));
	string name = line.substr(0, colonPos);
	std::transform(name.begin(), name.end(), name.begin(), tolower);
	if (name.find(' ', 0) != string::npos)
		throw(parseException(*this, 400, "Bad Request", "space before :"));
	string value = line.substr(colonPos + 1, string::npos);
	_splitHeaderField(value, _fields[name]);
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

void
HttpRequest::_checkHeader(void) throw(parseException)
{
	if (_fields["host"].size() == 0
	|| (_fields["host"].size() == 1 && _fields["host"][0] == ""))
		throw(parseException(*this, 400, "Bad Request", "no Host header field"));
	if (_fields["host"].size() > 1)
		throw(parseException(*this, 400, "Bad Request", "too many Host header fields"));
}

void
HttpRequest::_setRequiredFile(void)
{
	_extractQueryPart();
	if (_fileWithoutRoot[0] != '/')
		_fileWithoutRoot.insert(0, "/");
	if (_method == "PUT")
		_updatePutDirectory();
	else
	{
		_requiredFile = _getPath(_fileWithoutRoot);
		_addIndexIfDirectory();
		_updateStatusIfInvalid();
	}
}

void
HttpRequest::_extractQueryPart(void)
{
	size_t queryPos = _target.find('?');

	if (queryPos != string::npos)
		_queryPart = _target.substr(queryPos + 1);
	_fileWithoutRoot = _target.substr(0, queryPos);
}

string
HttpRequest::_getPath(string file) const
{
	string originalFile(file);
	std::map<string, map<string, vector<string> > >::iterator its = _host.location.begin();
	std::map<string, map<string, vector<string> > >::iterator ite = _host.location.end();
	std::map<string, map<string, vector<string> > >::iterator actual;
	size_t slashPos;
	while (file.size())
	{
		for (actual = its; actual != ite; ++actual)
			if (actual->first == file)
			{
				if (actual->second.find("root") != actual->second.end())
					return (actual->second["root"][0] + originalFile);
				else if (actual->second.find("alias") != actual->second.end())
					return (originalFile.replace(0, file.size(), actual->second["alias"][0]));
			}

		slashPos = file.find_last_of('/');
		if (slashPos == 0 && file != "/")
			file.erase(slashPos + 1);
		else if (slashPos != string::npos)
			file.erase(slashPos);
		else
			file.clear();
	}
	return (_host.root + originalFile);
}

void
HttpRequest::_updatePutDirectory(void)
{
	string analyzedFile(_fileWithoutRoot);
	std::map<string, map<string, vector<string> > >::iterator its = _host.location.begin();
	std::map<string, map<string, vector<string> > >::iterator ite = _host.location.end();
	std::map<string, map<string, vector<string> > >::iterator actual;
	size_t slashPos;
	while (analyzedFile.size())
	{
		for (actual = its; actual != ite; ++actual)
			if (actual->first == analyzedFile)
			{
				if (actual->second.find("upload_store") != actual->second.end())
				{
					_requiredFile = _fileWithoutRoot;
					_requiredFile.replace(0, analyzedFile.size(), actual->second["upload_store"][0]);
					return ;
				}
			}

		slashPos = analyzedFile.find_last_of('/');
		if (slashPos == 0 && analyzedFile != "/")
			analyzedFile.erase(slashPos + 1);
		else if (slashPos != string::npos)
			analyzedFile.erase(slashPos);
		else
			analyzedFile.clear();
	}
	_requiredFile = _getPath(_fileWithoutRoot);
}

void
HttpRequest::_addIndexIfDirectory(void)
{
	struct stat fileInfos;
	if (stat(_requiredFile.c_str(), &fileInfos) == 0
	&& S_ISDIR(fileInfos.st_mode))
	{
		if (_requiredFile[_requiredFile.size() - 1] != '/')
			_requiredFile += '/';
		if (_fileWithoutRoot[_fileWithoutRoot.size() - 1] != '/')
			_fileWithoutRoot += '/';
		if (_searchForIndexInLocations())
			return ;
		_searchForIndexInHost();
	}
}

bool
HttpRequest::_searchForIndexInLocations(void)
{
	struct stat fileInfos;
	string analyzedFile(_fileWithoutRoot);
	std::map<string, map<string, vector<string> > >::iterator its = _host.location.begin();
	std::map<string, map<string, vector<string> > >::iterator ite = _host.location.end();
	std::map<string, map<string, vector<string> > >::iterator actual;
	size_t slashPos;
	while (analyzedFile.size())
	{
		for (actual = its; actual != ite; ++actual)
			if (actual->first == analyzedFile)
			{
				if (actual->second.find("index") != actual->second.end())
				{
					vector<string> indexFiles = actual->second["index"];
					for (vector<string>::iterator indexFile = indexFiles.begin();
					indexFile != indexFiles.end(); ++indexFile)
					{
						string testedFile = _requiredFile + *indexFile;
						if (stat(testedFile.c_str(), &fileInfos) == 0)
						{
							_requiredFile = testedFile;
							_fileWithoutRoot += *indexFile;
							return (true);
						}
					}
					_requiredFile.clear();
					return (true);
				}
			}

		slashPos = analyzedFile.find_last_of('/');
		if (slashPos == 0 && analyzedFile != "/")
			analyzedFile.erase(slashPos + 1);
		else if (slashPos != string::npos)
			analyzedFile.erase(slashPos);
		else
			analyzedFile.clear();
	}
	return (false);
}

void
HttpRequest::_searchForIndexInHost(void)
{
	struct stat fileInfos;
	vector<string>::iterator index = _host.index.begin();
	vector<string>::iterator indexEnd = _host.index.end();
	for (; index != indexEnd; ++index)
	{
		string testedFile = _requiredFile + *index;
		if (stat(testedFile.c_str(), &fileInfos) == 0)
		{
			_requiredFile = testedFile;
			break ;
		}
	}
	if (index == indexEnd)
		_requiredFile.clear();
}

void
HttpRequest::_updateStatusIfInvalid(void)
{
	struct stat fileInfos;
	if (stat(_requiredFile.c_str(), &fileInfos) != 0 || !S_ISREG(fileInfos.st_mode))
		setStatus(404, "Not Found");
}

bool
HttpRequest::_methodIsAuthorized(void) const
{
	vector<string> const & allowedMethods = _getAllowedMethods();
	return (std::find(allowedMethods.begin(), allowedMethods.end(), _method) != allowedMethods.end());
}

vector<string> const
HttpRequest::_getAllowedMethods(void) const
{
	string analyzedFile(_fileWithoutRoot);
	size_t slashPos;
	std::map<string, map<string, vector<string> > >::iterator its = _host.location.begin();
	std::map<string, map<string, vector<string> > >::iterator ite = _host.location.end();
	std::map<string, map<string, vector<string> > >::iterator actual;

	while (analyzedFile.size())
	{
		for (actual = its; actual != ite; ++actual)
			if (actual->first == analyzedFile)
				if (actual->second.find("allowed_methods") != actual->second.end())
					return (actual->second["allowed_methods"]);

		slashPos = analyzedFile.find_last_of('/');
		if (slashPos == 0 && analyzedFile != "/")
			analyzedFile.erase(slashPos + 1);
		else if (slashPos != string::npos)
			analyzedFile.erase(slashPos);
		else
			analyzedFile.clear();
	}
	return (vector<string>());
}

void
HttpRequest::_setRequiredRealm(void)
{
	string analyzedFile(_fileWithoutRoot);
	std::map<string, map<string, vector<string> > >::iterator its = _host.location.begin();
	std::map<string, map<string, vector<string> > >::iterator ite = _host.location.end();
	std::map<string, map<string, vector<string> > >::iterator actual;
	size_t slashPos;

	while (analyzedFile.size())
	{
		for (actual = its; actual != ite; ++actual)
			if (actual->first == analyzedFile)
			{
				if (actual->second.find("auth_basic") != actual->second.end())
				{
					_requiredRealm.name = actual->second["auth_basic"][0];
					_requiredRealm.userFile = actual->second["auth_basic_user_file"][0];
					return ;
				}
			}

		slashPos = analyzedFile.find_last_of('/');
		if (slashPos == 0 && analyzedFile != "/")
			analyzedFile.erase(slashPos + 1);
		else if (slashPos != string::npos)
			analyzedFile.erase(slashPos);
		else
			analyzedFile.clear();
	}
}

void
HttpRequest::_setClientInfos(void) const
{
	vector<string> value;
	try { value = _fields.at("authorization"); }
	catch (std::out_of_range) { return ; }
	if (value.size() != 1 || std::count(value[0].begin(), value[0].end(), ' ') != 1)
		throw (parseException(*this, 401, "Unauthorized", "Invalid format of authorization header field"));

	size_t spacePos = value[0].find(' ');
	_client.authentications[_requiredRealm.name].scheme = string(value[0], 0, spacePos);
	string credentials = base64_decode(string(value[0], spacePos + 1));

	if (std::count(credentials.begin(), credentials.end(), ':') == 0)
		throw (parseException(*this, 401, "Unauthorized", "No ':' in credentials"));
	size_t colonPos = credentials.find(':');
	_client.authentications[_requiredRealm.name].user = string(credentials, 0, colonPos);
	_client.authentications[_requiredRealm.name].ident = _client.authentications[_requiredRealm.name].user;
	_client.authentications[_requiredRealm.name].password = string(credentials, colonPos + 1);
}

bool
HttpRequest::_isAuthorized(void) const
{
	if (_requiredRealm.name.empty())
		return (true);
	if (_client.authentications.find(_requiredRealm.name) == _client.authentications.end())
		return (false);
	std::ifstream accessFile(_requiredRealm.userFile.c_str());
	if (!accessFile)
		throw(parseException(*this, 500, "Internal Server Error", "Could not open access file"));
	string line;
	while (getline(accessFile, line))
	{
		size_t colonPos = line.find(':');
		if (colonPos == string::npos)
			throw(parseException(*this, 500, "Internal Server Error", "No ':' in access file"));
		if (line.substr(0, colonPos) == _client.authentications[_requiredRealm.name].user)
		{
			accessFile.close();
			return (string(line, colonPos + 1) == md5(_client.authentications[_requiredRealm.name].password));
		}
	}
	accessFile.close();
	return (false);
}

void
HttpRequest::_analyseBody(void) throw(parseException)
{
	_body[0] = 0;
	if (_fields["content-length"].size() == 0)
		return ;
	_checkContentLength(_fields["content-length"]);
	std::istringstream(_fields["content-length"][0]) >> _bodySize;
	if (_bodySize > CLIENT_MAX_BODY_SIZE)
		throw(parseException(*this, 413, "Payload Too Large", "Content-Length too high"));
	else if (_bodySize == 0)
		return ;
	ssize_t recvRet = recv(_client.s, _body, _bodySize, 0);
	if (recvRet < 0)
		throw(parseException(*this, 500, "Internal Server Error", "recv error"));
	else if (static_cast<size_t>(recvRet) < _bodySize)
		throw(parseException(*this, 500, "Internal Server Error", "body smaller than given content length"));
	_body[_bodySize] = 0;
}

void
HttpRequest::_checkContentLength(vector<string> const & contentLengthField) const throw(parseException)
{
	if (contentLengthField.size() > 1)
		throw(parseException(*this, 400, "Bad Request", "Mutiple Content-Length fields"));
	else if (contentLengthField[0].size() > 9)
		throw(parseException(*this, 400, "Bad Request", "Content-Length field is too long"));
	else if (contentLengthField[0].find_first_not_of("0123456789") != string::npos)
		throw(parseException(*this, 400, "Bad Request", "Content-Length is not a number"));
}

void
HttpRequest::_debugFields(void)
{
	cerr << "Header received : " << endl;
	map<string, vector<string> >::iterator it = _fields.begin();
	map<string, vector<string> >::iterator ite = _fields.end();
	for (; it != ite; ++it)
	{
		cout << "[" << it->first << "] = ";
		vector<string>::iterator vit = it->second.begin();
		vector<string>::iterator vite = it->second.end();
		for (; vit != vite; ++vit)
			cout << *vit << " ";
		cout << endl;
	}
	cerr << "end of debug 'header received'" << endl;
}
