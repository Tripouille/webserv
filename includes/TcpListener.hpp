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
# define MAXMSG 512
typedef int SOCKET;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

class TcpListener
{
	public:
		class tcpException : public std::exception
		{
			public:
				tcpException(const char * str = "") throw() : _str(str) {}
				virtual ~tcpException(void) throw() {}
				virtual const char * what(void) const throw()
				{
					static string s = string(_str) + " : " + strerror(errno);
					return (s.c_str());
				}
			private:
				const char * _str;
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

		TcpListener(void);
		TcpListener(TcpListener const& other);
		TcpListener& operator=(TcpListener const& other);
		void _killSocket(SOCKET sock);
};

#endif