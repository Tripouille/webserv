#include "Answer.hpp"
#include <sstream>
#include <fstream>
#include <map>
#include <sys/stat.h>
#include <sys/socket.h>
using std::map;

/* Exception */

Answer::sendException::sendException(string str) throw()
						   : _str(str + " : " + strerror(errno))
{
}

Answer::sendException::~sendException(void) throw()
{
}

const char *
Answer::sendException::what(void) const throw()
{
	return (_str.c_str());
}

/* Answer */

Answer::Answer(SOCKET client, ServerConfig const & config) : _client(client), _config(config)
{
}

Answer::~Answer()
{
	deleteQ(_body);
}

Answer::Answer(Answer const & other) : _config(other._config)
{
	Answer::_copy(other);
}

Answer &
Answer::operator=(Answer const & other)
{
	if (this != &other)
		Answer::_copy(other);
	return (*this);
}

/* Public */

void
Answer::setBody(t_bufferQ const & body)
{
	_body = body;
}

void
Answer::getFile(string const & fileName) throw(sendException)
{
	s_buffer *	buffer;

	std::ifstream indexFile(fileName.c_str());
	if (!indexFile.is_open())
		throw(sendException("Could not open file " + fileName));
	do
	{
		buffer = new s_buffer(BUFFER_SIZE);
		try {indexFile.read(buffer->b, buffer->size);}
		catch (std::exception const &)
		{throw(sendException("Coud not read file " + fileName));}
		_body.push(buffer);
		buffer->occupiedSize = indexFile.gcount();
	} while (buffer->occupiedSize == buffer->size && !indexFile.eof());
	indexFile.close();
}

void
Answer::sendStatus(HttpRequest::s_status const & status)
	const throw(sendException)
{
	std::ostringstream oss;
	oss << HTTP_VERSION << " " << status.code << " " << status.info << "\r\n";
	_sendToClient(oss.str().c_str(), oss.str().size());
}

void
Answer::sendHeader(void) const throw(sendException)
{
	std::ostringstream headerStream;

	for (map<string, string>::const_iterator it = _fields.begin(); it != _fields.end(); ++it)
		headerStream << it->first << ": " << it->second << "\r\n";
	string header = headerStream.str();
	//cerr << "header sent : " << endl << header << endl;
	_sendToClient(header.c_str(), header.size());
}

void
Answer::sendEndOfHeader(void) const throw(sendException)
{
	_sendToClient("\r\n", 2);
}

void
Answer::sendAnswer(HttpRequest const & request) throw(sendException)
{
	/*for (map<string, string>::iterator it = _fields.begin(); it != _fields.end(); ++it)
		cerr << "_fields[" << it->first << "] = " << it-> second << endl;*/
	_fillServerField();
	_fillDateField();
	_fillContentFields(request._requiredFile);
	sendHeader();
	sendEndOfHeader();
	if (request._method != "HEAD")
		_sendBody();
	else
		deleteQ(_body);
}

/* Private */

void
Answer::_copy(Answer const & other)
{
	_client = other._client;
	_fields = other._fields;
	_body = other._body;
}

void
Answer::_sendToClient(char const * msg, size_t size)
	const throw(sendException)
{
	if (send(_client, msg, size, 0) == -1)
		throw(sendException("Could not send to client"));
}

void
Answer::_sendBody(void) throw(sendException)
{
	while (!_body.empty())
	{
		_sendToClient(_body.front()->b, static_cast<size_t>(_body.front()->occupiedSize));
		delete _body.front();
		_body.pop();
	}
}

void
Answer::_fillServerField(void)
{
	_fields["Server"] = "webserv";
}

void
Answer::_fillDateField(void)
{
	char date[50]; 
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	_fields["Date"] = string(date);
}

void
Answer::_fillContentFields(string const & fileName)
{
	string extension = fileName.substr(fileName.find_last_of('.') + 1, string::npos);
	if (_fields.count("content-type") == 0 && _config.mimeType.count(extension))
		_fields["Content-Type"] = _config.mimeType.at(extension);

	streamsize fileSize = static_cast<streamsize>(_body.size() - 1)
							* _body.back()->size + _body.back()->occupiedSize;
	std::ostringstream fileSizeStream; fileSizeStream << fileSize;
	_fields["Content-Length"] = fileSizeStream.str();

	struct stat fileInfos;
	stat(fileName.c_str(), &fileInfos);
	time_t lastModified = fileInfos.st_mtime;
	struct tm tm = *gmtime(&lastModified);
	char date[50];
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	_fields["Last-Modified"] = string(date);
}

void
Answer::_debugFields(void)
{
	cerr << "debugging fields of answer object :" << endl;
	map<string, string >::iterator it = _fields.begin();
	map<string, string >::iterator ite = _fields.end();
	for (; it != ite; ++it)
		cout << "[" << it->first << "] = " << it->second << endl;
	cerr << "end of debug" << endl;
}