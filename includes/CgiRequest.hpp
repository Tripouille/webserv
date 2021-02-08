#ifndef CGIREQUEST_HPP
# define CGIREQUEST_HPP

# include <cstdlib>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include <iostream>
# include <string.h>
# include <vector>
# include <signal.h>
# include <sys/socket.h>
# include <fcntl.h>
# include "BufferQ.hpp"
# include "HttpRequest.hpp"
# include "Client.hpp"
# include "Answer.hpp"
# include "ServerConfig.hpp"
# include "utils.hpp"
# define STDOUT 1
# define STDIN 0
# define UNSET_PIPE -1

typedef int SOCKET;

using std::cout; using std::endl; using std::string;

class CgiRequest
{
	friend class TcpListener;
	class cgiException : public std::exception
	{
		public:
			cgiException(string str, CgiRequest & cgiRequest) throw();
			virtual ~cgiException(void) throw();
			virtual const char * what(void) const throw();
		private:
			string _str;
	};

	public:
		CgiRequest(const unsigned short serverPort, HttpRequest & request, \
					Client const & client, string & cgi);
		~CgiRequest(void);

		void doRequest(Answer & answer);

	private:
		CgiRequest(void);
		CgiRequest(CgiRequest const & other);
		CgiRequest & operator=(CgiRequest const & other);
		void _setEnv(int pos, string const & value);
		void _setArg(int pos, string const & value);
		void _analyzeHeader(int fd, Answer & answer);
		ssize_t _getLine(int fd, char * buffer, ssize_t limit) const;
		void _parseHeaderLine(string line, Answer & answer) throw(cgiException);
		void _extractStatus(string & field) const;

		enum {BUFF_SIZE = 100000, TIMEOUT = 1000000, ENV_SIZE = 20};

		char *				_env[ENV_SIZE];
		char *				_av[2];
		string &			_cgi;
		SOCKET				_socket;
		int					_inPipe[2];
		int					_outPipe[2];
		HttpRequest &		_request;
};

#endif