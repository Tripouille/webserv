#ifndef TCPLISTENER_HPP
# define TCPLISTENER_HPP
# include <string>
# include <iostream>
# include <sys/socket.h>
# include <unistd.h>
# include <netinet/in.h>
# include <cstring>
# include <cerrno>
# define BACKLOG 3
using std::string;

class TcpListener
{
	public:
		class tcpException : public std::exception
		{
			public:
				tcpException(const char * str = "") throw() : _str(str) {}
				virtual ~tcpException(void) throw() {}
				virtual const char * what(void) const throw()
				{;
					std::cerr << strerror(errno) << " : ";
					return (_str);
				}
			private:
				const char * _str;
		};

		TcpListener(const char * ipAddress, int port);
		virtual ~TcpListener();

		void init(void);

	private:
		const char *	_address;
		const int		_port;
		int				_socket;
		const int		_backlog;

		TcpListener(void);
		TcpListener(TcpListener const& other);
		TcpListener& operator=(TcpListener const& other);
};

#endif