#ifndef TCPLISTENER_HPP
# define TCPLISTENER_HPP
# include <string>
# include <iostream>
# include <sys/socket.h>
# include <unistd.h>
# include <netinet/in.h>
# include <cstring>
# include <cerrno>
# include <vector>
# include <map>
# include <sstream>

# include "HttpRequest.hpp"

# define BACKLOG 3
# define CLIENT_MAX_BODY_SIZE 1000000
# define REQUEST_LINE_MAX_SIZE 1024
# define HEADER_MAX_SIZE 8000
# define HTTP_VERSION "HTTP/1.1"

typedef int SOCKET;

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::map;

class TcpListener
{
	public:
		class tcpException : public std::exception
		{
			public:
				tcpException(string str = "") throw();
				virtual ~tcpException(void) throw();
				virtual const char * what(void) const throw();
			private:
				string _str;
		};

		TcpListener(in_addr_t const & ipAddress, uint16_t port);
		virtual ~TcpListener();

		void init(void);
		void run(void);

	private:
		in_addr_t		_ipAddress;
		const uint16_t	_port;
		int				_socket;
		const int		_backlog;
		fd_set			_activeFdSet;
		int				_clientNb;

		TcpListener(void);
		TcpListener(TcpListener const& other);
		TcpListener& operator=(TcpListener const& other);
		void _disconnectClient(SOCKET client);
		void _acceptNewClient(void) throw(tcpException);
		void _receiveData(SOCKET client);
		s_request _parseRequest(char * buffer, s_status & status) const throw(parseException);
		void _parseRequestLine(std::istringstream & iss, s_request & request,
								s_status & status) const throw(parseException);
		vector<string> _split(string s, char delim) const;
		void _sendStatus(SOCKET client, s_status const & status);
		void _checkMethod(string const & method, s_status & status) const throw(parseException);
		void _checkTarget(string const & target, s_status & status) const throw(parseException);
		void _checkHttpVersion(string const & httpVersion, s_status & status) const throw(parseException);
};

#endif