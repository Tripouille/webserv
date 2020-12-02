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
		void _sendStatus(SOCKET client, HttpRequest::s_status const & status);
};

#endif