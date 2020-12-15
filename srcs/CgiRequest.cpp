#include "CgiRequest.hpp" 

CgiRequest::CgiRequest(void)
{
	_env[0] = strdup("GATEWAY_INTERFACE=CGI/1.1");
	_env[1] = strdup("SERVER_PROTOCOL=HTTP/1.1");
	_env[2] = strdup("SCRIPT_FILENAME=./test.php");
	_env[3] = strdup("SCRIPT_NAME=test.php");
	_env[4] = strdup("REDIRECT_STATUS=200");
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
	int stdOutSave = dup(1);
	int p[2]; pipe(p);
	int child = fork();
	if (child == 0)
	{
		dup2(p[1], 1);
		if (execve("./cgitest/printenv", _av, _env) == -1)
			exit(EXIT_FAILURE);
	}
	else
	{
		usleep(100000);
		waitpid(child, &status, 1);
		kill(child, SIGKILL);

		ssize_t readReturn = read(p[0], _buffer, BUFFER_SIZE);
		_buffer[readReturn] = 0;
		dup2(stdOutSave, 1);
		cout << _buffer << endl;
	}
}

/* Private method */

void
CgiRequest::_copy(CgiRequest const & other)
{
	static_cast<void>(other);
}
