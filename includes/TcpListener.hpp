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
# include <sys/stat.h>
# include <algorithm>

# include "utils.hpp"
# include "HttpRequest.hpp"
# include "CgiRequest.hpp"
# include "BufferQ.hpp"
# include "Client.hpp"
# include "Answer.hpp"
# include "ServerConfig.hpp"
# include "thread.hpp"

# define BACKLOG 5000
# define RCV_TIMEOUT 1

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
		void handleSocket(size_t id, fd_set const & readfds, fd_set const & writefds, int workerNb);

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
		pthread_mutex_t			_clientMutex;

		TcpListener(void);
		TcpListener(TcpListener const & other);
		TcpListener & operator=(TcpListener const & other);

		void _acceptNewClient(void) throw(tcpException);
		void _disconnectClient(SOCKET client);
		void _handleRequest(SOCKET client) throw(tcpException);
		void _listDirectory(HttpRequest & request, Answer & answer) const;
		void _writeInFile(HttpRequest & request) const;
		void _writeInFile(HttpRequest & request, t_bufferQ body) const;
		void _answerToClient(SOCKET client, Answer & answer,
			HttpRequest & request)
			throw(tcpException,  Answer::sendException);
		string const _getCgiPath(string const & fileName) const;
		void _doCgiRequest(CgiRequest cgiRequest, Answer & answer) const;
		void _setErrorFields(HttpRequest const & request, Answer & answer) const;
		bool _setErrorPage(HttpRequest & request) const;
		void _handleNoErrorPage(Answer & answer, HttpRequest const & request);

		enum { DEFAULT_WORKER_NB = 8 };
};

#endif
