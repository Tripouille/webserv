#ifndef CGIREQUEST_HPP
# define CGIREQUEST_HPP

#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <signal.h>
#include <sys/socket.h>
#include "BufferQ.hpp"
#include "HttpRequest.hpp"
#define STDOUT 1

using std::cout; using std::endl; using std::string;

class CgiRequest
{
	class cgiException : public std::exception
	{
		public:
			cgiException(string str = "") throw();
			virtual ~cgiException(void) throw();
			virtual const char * what(void) const throw();
		private:
			string _str;
	};
	public:
		CgiRequest(const uint16_t serverPort, HttpRequest const & request,
						string const & requiredFile);
		~CgiRequest(void);
		CgiRequest(CgiRequest const & other);

		CgiRequest & operator=(CgiRequest const & other);
		void doRequest(void);
		t_bufferQ const & getAnswer(void) const;

	private:
		CgiRequest(void);
		void _copy(CgiRequest const & other);
		void _setEnv(int pos, string const & value);
		template <class T>
		string _toString(T number) const;


		enum {BUFFER_SIZE = 100000, TIMEOUT = 1000000, ENV_SIZE = 20};

		char *				_env[ENV_SIZE];
		char *				_av[2];
		t_bufferQ			_answer;	
};

#endif

/*
AUTH_TYPE      = "" | auth-scheme
      			auth-scheme    = "Basic" | "Digest" | extension-auth
      			extension-auth = token
For HTTP, if the client request required authentication for external
   access, then the server MUST set the value of this variable from the
   'auth-scheme' token in the request Authorization header field.

CONTENT_LENGTH
The CONTENT_LENGTH variable contains the size of the message-body
   attached to the request, if any, in decimal number of octets.  If no
   data is attached, then NULL (or unset).

      CONTENT_LENGTH = "" | 1*digit

   The server MUST set this meta-variable if and only if the request is
   accompanied by a message-body entity.  The CONTENT_LENGTH value must
   reflect the length of the message-body after the server has removed
   any transfer-codings or content-codings.
CONTENT_TYPE
GATEWAY_INTERFACE
PATH_INFO
PATH_TRANSLATED
QUERY_STRING
REMOTE_ADDR
REMOTE_IDENT
REMOTE_USER
REQUEST_METHOD
REQUEST_URI
SCRIPT_NAME
SERVER_NAME
SERVER_PORT
SERVER_PROTOCOL
SERVER_SOFTWARE
*/