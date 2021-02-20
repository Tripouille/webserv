#include "HttpRequest.hpp"
#include <sys/socket.h>

/* Exceptions */

HttpRequest::parseException::parseException(HttpRequest const & request,
				 int code, string const & info, string errorMsg) throw()
			: _str(info + " : " + errorMsg)
{
	const_cast<HttpRequest &>(request).setStatus(code, info);
	while (true)
	{
		std::streamsize recvReturn = recv(request._client.s, NULL, CLIENT_MAX_BODY_SIZE, MSG_DONTWAIT);
		if (recvReturn <= 0)
			return ;
	}
}

HttpRequest::parseException::~parseException(void) throw()
{
}

const char *
HttpRequest::parseException::what(void) const throw()
{
	return (_str.c_str());
}

HttpRequest::directoryListingException::directoryListingException(void) throw()
{
}

HttpRequest::directoryListingException::~directoryListingException(void) throw()
{
}

const char *
HttpRequest::directoryListingException::what(void) const throw()
{
	return ("Directory Listing");
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
	_body = new char[CLIENT_MAX_BODY_SIZE + 1];
}

HttpRequest::~HttpRequest(void)
{
	delete[] _body;
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
HttpRequest::analyze(void) throw(parseException, closeOrderException, directoryListingException)
{
	_headerSize = 0;
	_analyseRequestLine();
	_analyseHeader();
	//_debugFields();
	_setRequiredFile();
	_analyseBody();

	if (!_methodIsAuthorized())
		throw(parseException(*this, 405, "Method Not Allowed", "from config"));
	_setRequiredRealm();
	if (_requiredRealm.name.size())
		_setClientInfos();
	if (!_isAuthorized())
		throw(parseException(*this, 401, "Unauthorized", "wrong credentials"));
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
HttpRequest::_analyseRequestLine(void) throw(parseException, closeOrderException)
{
	char			buffer[REQUEST_LINE_MAX_SIZE + 1];
	vector<string>	requestLine;

	for (int i = 0; i <= atoi(_config.http.at("max_empty_line_before_request").c_str())
	&& (((_headerSize = _getLine(buffer, REQUEST_LINE_MAX_SIZE)) == 2 && buffer[0] == 0)
	|| _headerSize == 1); ++i)
		;
	if ((_headerSize == 2 && buffer[0] == 0)
	|| _headerSize == 1)
		throw(parseException(*this, 400, "Bad Request", "Too many empty lines before request"));
	if (_headerSize < 0)
		throw(parseException(*this, 500, "Internal Server Error", "recv error (may have timeout)"));
	else if (_headerSize == 0)
		throw(closeOrderException());
	else if (_headerSize > REQUEST_LINE_MAX_SIZE)
		throw(parseException(*this, 431, "Request Line Too Long", "request line too long"));

	requestLine = _splitRequestLine(buffer);
	if (requestLine.size() != 3)
		throw parseException(*this, 400, "Bad Request", "invalid request line : "
		+ string(buffer));
	_fillAndCheckRequestLine(requestLine);

	cout << "Request line : " << buffer << endl;
}

ssize_t
HttpRequest::_getLine(char * buffer, ssize_t limit, bool LFAllowed) const throw(parseException)
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
	if (!LFAllowed && (lineSize < 2 || buffer[lineSize - 2] != '\r'))
		throw(parseException(*this, 400, "Bad Request", "Missing CRLF"));
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
HttpRequest::_analyseHeader(void) throw(parseException)
{
	//+1 pour pouvoir lire un char supplémentaire et dépasser la limite
	char			line[HEADER_MAX_SIZE + 1];
	ssize_t			lineSize;

	while (_headerSize <= HEADER_MAX_SIZE
	&& (lineSize = _getLine(line, HEADER_MAX_SIZE)) > 0
	&& line[0])
	{
		_headerSize += lineSize;
		_parseHeaderLine(line);
	}
	if (lineSize < 0)
		throw(parseException(*this, 500, "Internal Server Error", "recv error"));
	else if (_headerSize > HEADER_MAX_SIZE)
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
HttpRequest::_analyseBody(void) throw(parseException)
{
	_body[0] = 0;
	int maxBodySize = CLIENT_MAX_BODY_SIZE;
	string analyzedFile(_fileWithoutRoot);
	map<string, vector<string> > location = _getDeepestLocation("client_max_body_size", analyzedFile);
	if (!location.empty())
	{
		maxBodySize = atoi(location["client_max_body_size"][0].c_str());
		if (maxBodySize < 0 || location["client_max_body_size"][0].size() > 9)
		{
			cerr << "client_max_body_size config ignored" << endl;
			maxBodySize = CLIENT_MAX_BODY_SIZE;
		}
		if (maxBodySize == 0)
			maxBodySize = CLIENT_MAX_BODY_SIZE;
	}
	if (_fields["transfer-encoding"].size() == 1 && _fields["transfer-encoding"][0] == "chunked")
		_analyseChunkedBody(maxBodySize);
	else if (_fields["content-length"].size())
		_analyseNormalBody(maxBodySize);
}

void
HttpRequest::_analyseChunkedBody(int maxBodySize) throw(parseException)
{
	ssize_t			chunkSizeBufferMaxSize = static_cast<ssize_t>(intToHex(CLIENT_MAX_BODY_SIZE).size());
	ssize_t			chunkSizeBufferSize = 0;
	char			chunkSizeBuffer[chunkSizeBufferMaxSize + 1];
	ssize_t			chunkSize = 0, chunkLineSize = 0;
	char			crlf[3] = {0};

	do
	{
		chunkSizeBufferSize = _getLine(chunkSizeBuffer, chunkSizeBufferMaxSize, false);
		if (chunkSizeBufferSize < 0)
			throw(parseException(*this, 500, "Internal Server Error", "recv error"));
		else if (chunkSizeBufferSize > chunkSizeBufferMaxSize)
			throw(parseException(*this, 413, "Payload Too Large", "Chunk size is too large"));
		else if (!chunkSizeBuffer[0])
			throw(parseException(*this, 400, "Bad Request", "Missing chunk size"));
		else if (!isHex(chunkSizeBuffer))
			throw(parseException(*this, 400, "Bad Request", "Chunk size is not hex"));
		chunkSizeBuffer[chunkSizeBufferSize] = 0;
		chunkSize = hexToDec(chunkSizeBuffer);
		if (static_cast<ssize_t>(_bodySize) + chunkSize > maxBodySize)
			throw(parseException(*this, 413, "Payload Too Large", "Chunked body is too large"));
		else if (chunkSize)
		{
			chunkLineSize = loopRecv(_client.s, _body + _bodySize, chunkSize);
			if (chunkLineSize < 0)
				throw(parseException(*this, 500, "Internal Server Error", "recv error"));
			else if (chunkLineSize != chunkSize)
				throw(parseException(*this, 500, "Internal Server Error", "Chunked body smaller than chunk size"));
			_bodySize += static_cast<size_t>(chunkSize);
		}
		chunkLineSize = loopRecv(_client.s, crlf, 2);
		if (chunkLineSize < 0)
			throw(parseException(*this, 500, "Internal Server Error", "recv error"));
		crlf[chunkLineSize] = 0;
		if (strcmp(crlf, "\r\n") != 0)
			throw(parseException(*this, 400, "Bad Request", "No CRLF at the end of a chunk"));
	} while (chunkSize);
	//_body[_bodySize] = 0;//for debug only
	//cout << "CHUNK body size = " << _bodySize << ", body = [" << _body << "]" << endl;
}

void
HttpRequest::_analyseNormalBody(int maxBodySize) throw(parseException)
{
	_checkContentLength(_fields["content-length"]);
	std::istringstream(_fields["content-length"][0]) >> _bodySize;
	if (_bodySize > static_cast<size_t>(maxBodySize))
		throw(parseException(*this, 413, "Payload Too Large", "Content-Length too high"));
	else if (_bodySize == 0)
		return ;
	ssize_t recvRet = loopRecv(_client.s, _body, static_cast<ssize_t>(_bodySize));
	if (recvRet < 0)
		throw(parseException(*this, 500, "Internal Server Error", "recv error"));
	else if (static_cast<size_t>(recvRet) < _bodySize)
		throw(parseException(*this, 500, "Internal Server Error", "body smaller than given content length (" + toStr(recvRet) + ")"));
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

map<string, vector<string> >
HttpRequest::_getDeepestLocation(string const & key, string & analyzedFile) const
{
	std::map<string, map<string, vector<string> > >::iterator its = _host.location.begin();
	std::map<string, map<string, vector<string> > >::iterator ite = _host.location.end();
	std::map<string, map<string, vector<string> > >::iterator actual;
	size_t slashPos;

	while (analyzedFile.size())
	{
		for (actual = its; actual != ite; ++actual)
			if (actual->first == analyzedFile)
				if (actual->second.find(key) != actual->second.end())
					return (actual->second);
		slashPos = analyzedFile.find_last_of('/');
		if (slashPos == 0 && analyzedFile != "/")
			analyzedFile.erase(slashPos + 1);
		else if (slashPos != string::npos)
			analyzedFile.erase(slashPos);
		else
			analyzedFile.clear();
	}
	return (map<string, vector<string> >());
}

void
HttpRequest::_setRequiredFile(void)
{
	_extractQueryPart();
	if (_fileWithoutRoot[0] != '/')
		_fileWithoutRoot.insert(0, "/");
	if (_method != "PUT")
	{
		_requiredFile = _getPath(_fileWithoutRoot);
		_addIndexIfDirectory();
		if (!(_extensionPart = _getLanguageAndEncodingExtension()).empty())
			_requiredFile += _extensionPart;
		_fileFound = _updateStatusIfInvalid();
	}
	if (_method == "PUT" || (_method == "POST" && !_fileFound))
		_useStoreDirectory();
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
		if (_searchForIndexInHost())
			return ;
		if (_directoryListingIsActivated())
			throw(directoryListingException());
		_requiredFile.clear();
	}
}

bool
HttpRequest::_searchForIndexInLocations(void)
{
	struct stat fileInfos;
	string analyzedFile = _fileWithoutRoot;
	map<string, vector<string> > location = _getDeepestLocation("index", analyzedFile);
	if (!location.empty())
	{
		vector<string> indexFiles = location["index"];
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
	return (false);
}

bool
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
			return (true);
		}
	}
	return (false);
}

bool
HttpRequest::_directoryListingIsActivated(void) const
{
	string analyzedFile = _fileWithoutRoot;
	map<string, vector<string> > location = _getDeepestLocation("autoindex", analyzedFile);
	if (!location.empty())
		return (location["autoindex"][0] == "on");
	else if (_host.autoIndex)
		return (true);
	return (false);
}

struct compareExtensions
{
    string language, charset;
    compareExtensions(string l, string c) : language(l), charset(c) {}
    bool operator()(vector<string> const & extensions)
	{
		string firstFileExtension = extensions[0];
		size_t firstDashPos = firstFileExtension.find('-');
		if (language == "")
			return (extensions.size() == 1 && charset == firstFileExtension);
		if (charset == "")
			return (extensions.size() == 1
					&& (language == firstFileExtension || language == firstFileExtension.substr(0, firstDashPos)));
		if (extensions.size() == 1)
			return (false);
		string secondFileExtension = extensions[1];
		size_t secondDashPos = secondFileExtension.find('-');
		if (language == "*" && (charset == firstFileExtension || charset == secondFileExtension))
			return (true);
		if (charset == "*" &&
		((language == firstFileExtension || language == firstFileExtension.substr(0, firstDashPos))
		|| (language == secondFileExtension || language == secondFileExtension.substr(0, secondDashPos))))
			return (true);
		if ((language == firstFileExtension || language == firstFileExtension.substr(0, firstDashPos))
		&& charset == secondFileExtension)
			return (true);
		if (charset == firstFileExtension
		&& (language == secondFileExtension || language == secondFileExtension.substr(0, secondDashPos)))
			return (true);
		return (false);
	}
};

string
HttpRequest::_getLanguageAndEncodingExtension(void)
{
	struct stat fileInfos;
	if (stat(_requiredFile.c_str(), &fileInfos) != 0)
	{
		vector<vector<string> > extensionsInDirectory = _getVariantFilesInDirectory();
		if (extensionsInDirectory.empty())
			return ("");

		vector<std::pair<string, double> > acceptedLanguages = _getAcceptedExtensions("accept-language");
		vector<std::pair<string, double> > acceptedCharsets = _getAcceptedExtensions("accept-charset");

		// Search for the best file according to accepted languages and charsets
		vector<vector<string> >::iterator fileIt;
		for (vector<std::pair<string, double> >::iterator languageIt = acceptedLanguages.begin();
		languageIt != acceptedLanguages.end(); ++languageIt)
		{
			for (vector<std::pair<string, double> >::iterator charsetIt = acceptedCharsets.begin();
			charsetIt != acceptedCharsets.end(); ++charsetIt)
			{
				fileIt = std::find_if(extensionsInDirectory.begin(), extensionsInDirectory.end(),
																			compareExtensions(languageIt->first, charsetIt->first));
				if (fileIt != extensionsInDirectory.end())
					return ('.' + (*fileIt)[0] + '.' + (*fileIt)[1]);
			}
			fileIt = std::find_if(extensionsInDirectory.begin(), extensionsInDirectory.end(),
																		compareExtensions(languageIt->first, ""));
			if (fileIt != extensionsInDirectory.end())
				return ('.' + (*fileIt)[0]);
			fileIt = std::find_if(extensionsInDirectory.begin(), extensionsInDirectory.end(),
																		compareExtensions(languageIt->first, "*"));
			if (fileIt != extensionsInDirectory.end())
				return ('.' + (*fileIt)[0] + '.' + (*fileIt)[1]);
		}
		for (vector<std::pair<string, double> >::iterator charsetIt = acceptedCharsets.begin();
		charsetIt != acceptedCharsets.end(); ++charsetIt)
		{
			fileIt = std::find_if(extensionsInDirectory.begin(), extensionsInDirectory.end(),
																		compareExtensions("", charsetIt->first));
			if (fileIt != extensionsInDirectory.end())
				return ('.' + (*fileIt)[0]);
			fileIt = std::find_if(extensionsInDirectory.begin(), extensionsInDirectory.end(),
																		compareExtensions("*", charsetIt->first));
			if (fileIt != extensionsInDirectory.end())
				return ('.' + (*fileIt)[0] + '.' + (*fileIt)[1]);
		}
		return ('.' + extensionsInDirectory[0][0] +
										(extensionsInDirectory[0].size() == 2 ? extensionsInDirectory[0][1] : ""));
	}
	return ("");
}

static bool
extensionComp(vector<string> const & v1, vector<string> const & v2)
{
	return (v1.size() < v2.size());
}

vector<vector<string> >
HttpRequest::_getVariantFilesInDirectory(void)
{
	struct stat fileInfos;
	vector<vector<string> > extensionsInDirectory;
	string directoryName(_requiredFile);
	size_t slashPos = _requiredFile.find_last_of('/');
	directoryName.erase(slashPos == string::npos ? 0 : slashPos);
	string baseFileName(_requiredFile);
	baseFileName.erase(0, directoryName.size() + 1);
	DIR *dir;
	if ((dir = opendir(directoryName.c_str())) != NULL)
	{
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
		{
			string fileName = ent->d_name;
			if (stat((directoryName + '/' + ent->d_name).c_str(), &fileInfos) == 0 && S_ISREG(fileInfos.st_mode)
			&& fileName.substr(0, baseFileName.size() + 1) == baseFileName + '.')
			{
				string extensionsString = fileName.substr(baseFileName.size() + 1);
				vector<string> extensions;
				size_t pointPos = extensionsString.find('.');
				extensions.push_back(extensionsString.substr(0, pointPos));
				if (pointPos != string::npos)
					extensions.push_back(extensionsString.substr(pointPos + 1));
				extensionsInDirectory.push_back(extensions);
			}
		}
		closedir(dir);
	}
	std::sort(extensionsInDirectory.begin(), extensionsInDirectory.end(), extensionComp);
	return (extensionsInDirectory);
}

static bool
priorityComp(std::pair<string, double> const & p1, std::pair<string, double> const & p2)
{
	if (p1.second == p2.second)
		return (p1.first.size() < p2.first.size());
	return (p1.second >= p2.second);
}

vector<std::pair<string, double> >
HttpRequest::_getAcceptedExtensions(string const & fieldKey)
{
	vector<std::pair<string, double> > acceptedExtensions;
	if (_fields.find(fieldKey) != _fields.end())
	{
		for (vector<string>::iterator it = _fields[fieldKey].begin();
		it != _fields[fieldKey].end(); ++it)
		{
			string extension = it->substr(0, it->find(';'));
			double priority = 1.0;
			size_t priorityPos = it->find(";q=");
			if (priorityPos != string::npos)
				priority = atof(it->substr(priorityPos + 3).c_str());
			acceptedExtensions.push_back(std::make_pair(extension, priority));
		}
		std::sort(acceptedExtensions.begin(), acceptedExtensions.end(), priorityComp);
	}
	return (acceptedExtensions);
}

bool
HttpRequest::_updateStatusIfInvalid(void)
{
	struct stat fileInfos;
	if (stat(_requiredFile.c_str(), &fileInfos) != 0 || !S_ISREG(fileInfos.st_mode))
	{
		if (_method != "PUT" && _method != "POST")
			setStatus(404, "Not Found");
		return (false);
	}
	return (true);
}

void
HttpRequest::_useStoreDirectory(void)
{
	string analyzedFile(_fileWithoutRoot);
	map<string, vector<string> > location = _getDeepestLocation("upload_store", analyzedFile);
	if (!location.empty())
	{
		_requiredFile = _fileWithoutRoot;
		_requiredFile.replace(0, analyzedFile.size(), location["upload_store"][0]);
	}
	else if (_method == "PUT")
		_requiredFile = _getPath(_fileWithoutRoot);
}

bool
HttpRequest::_methodIsAuthorized(void) const
{
	string analyzedFile = _fileWithoutRoot;

	//temporary:
	string extension = _fileWithoutRoot.substr(_fileWithoutRoot.find_last_of('.') + 1);
	if (extension == "bla" && _method == "POST")
		return (true);

	map<string, vector<string> > location = _getDeepestLocation("allowed_methods", analyzedFile);
	if (!location.empty())
		return (std::find(location["allowed_methods"].begin(), location["allowed_methods"].end(), _method)
				!= location["allowed_methods"].end());
	return (false);
}

void
HttpRequest::_setRequiredRealm(void)
{
	string analyzedFile = _fileWithoutRoot;
	map<string, vector<string> > location = _getDeepestLocation("auth_basic", analyzedFile);
	if (!location.empty())
	{
		_requiredRealm.name = location["auth_basic"][0];
		_requiredRealm.userFile = location["auth_basic_user_file"][0];
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
HttpRequest::_debugFields(void)
{
	cout << "Header received : " << endl;
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
	cout << "end of debug 'header received'" << endl;
}
