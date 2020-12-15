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
	_env[0] = const_cast<char *>("GATEWAY_INTERFACE=CGI/1.1");
	_env[1] = const_cast<char *>("SERVER_PROTOCOL=HTTP/1.1");
	_env[2] = const_cast<char *>("SCRIPT_FILENAME=./cgitest/test.php");
	_env[3] = const_cast<char *>("SCRIPT_NAME=./cgitest/test.php");
	_env[4] = const_cast<char *>("REDIRECT_STATUS=200");
	_env[5] = NULL;

	_av[0] = NULL;
}

CgiRequest::~CgiRequest(void)
{
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
