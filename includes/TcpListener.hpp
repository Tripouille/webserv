#ifndef TCPLISTENER_HPP
# define TCPLISTENER_HPP
# include <string>
# include <iostream>
# include <sys/socket.h>
# include <unistd.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <cstring>
# include <cerrno>
# include <vector>
# include <map>
# include <queue>
# include <list>
# include <sstream>

# include "HttpRequest.hpp"
# include "CgiRequest.hpp"
# include "BufferQ.hpp"
# include "Client.hpp"
# include "Answer.hpp"
# include "ServerConfig.hpp"

# define BACKLOG 3
# define RCV_TIMEOUT 3000

typedef int SOCKET;

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::queue;
using std::list;
using std::map;
using std::streamsize;

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

		TcpListener(in_addr_t const & ipAddress, uint16_t port, \
			ServerConfig & config, Host & host);
		virtual ~TcpListener();

		void init(void);
		void run(void);

	private:
		in_addr_t				_ipAddress;
		uint16_t				_port;
		int						_socket;
		const int				_backlog;
		fd_set					_activeFdSet;
		int						_clientNb;
		map<SOCKET, Client> 	_clientInfos;
		ServerConfig &			_config;
		Host &					_host;


		TcpListener(void);
		TcpListener(TcpListener const & other);
		TcpListener & operator=(TcpListener const & other);

		void _acceptNewClient(void) throw(tcpException);
		void _disconnectClient(SOCKET client);
		void _handleRequest(SOCKET client) throw(tcpException);
		void _answerToClient(SOCKET client, HttpRequest & request)
			throw(tcpException,  Answer::sendException);
		void _handleBadStatus(Answer & answer, HttpRequest const & request)
			throw(Answer::sendException);
};

#endif
