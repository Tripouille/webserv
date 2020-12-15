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

CgiRequest::CgiRequest(string auth_type, string content_length, string content_type, string gateway_interface,
					string path_info, string path_translated, string query_string, string remote_addr,
					string remote_ident, string remote_user, string request_method, string request_uri,
					string script_name, string server_name, string server_port, string server_protocol,
					string server_software)
{
	_setEnv(0, auth_type.insert(0, "AUTH_TYPE="));
	_setEnv(1, content_length.insert(0, "CONTENT_LENGTH="));
	_setEnv(2, content_type.insert(0, "CONTENT_TYPE="));
	_setEnv(3, gateway_interface.insert(0, "GATEWAY_INTERFACE="));
	_setEnv(4, path_info.insert(0, "PATH_INFO="));
	_setEnv(5, path_translated.insert(0, "PATH_TRANSLATED="));
	_setEnv(6, query_string.insert(0, "QUERY_STRING="));
	_setEnv(7, remote_addr.insert(0, "REMOTE_ADDR="));
	_setEnv(8, remote_ident.insert(0, "REMOTE_IDENT="));
	_setEnv(9, remote_user.insert(0, "REMOTE_USER="));
	_setEnv(10, request_method.insert(0, "REQUEST_METHOD="));
	_setEnv(11, request_uri.insert(0, "REQUEST_URI="));
	_setEnv(12, script_name.insert(0, "SCRIPT_NAME="));
	_setEnv(13, server_name.insert(0, "SERVER_NAME="));
	_setEnv(14, server_port.insert(0, "SERVER_PORT="));
	_setEnv(15, server_protocol.insert(0, "SERVER_PROTOCOL="));
	_setEnv(16, server_software.insert(0, "SERVER_SOFTWARE="));
	_env[17] = NULL;
	_av[0] = NULL;
}

CgiRequest::~CgiRequest(void)
{
	for (int i = 0; _env[i] != NULL; ++i)
		delete _env[i];
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
		if (execve("/usr/bin/php-cgi", _av, _env) == -1)
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
	_env[value.size()] = 0;
}