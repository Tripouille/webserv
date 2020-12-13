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
# include <queue>
# include <list>
# include <sstream>

# include "HttpRequest.hpp"

# define BACKLOG 3
# define HTTP_VERSION "HTTP/1.1"
# define BUFFER_SIZE 1024

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
		class sendException : public std::exception
		{
			public:
				sendException(string str = "") throw();
				virtual ~sendException(void) throw();
				virtual const char * what(void) const throw();
			private:
				string _str;
		};

		TcpListener(in_addr_t const & ipAddress, uint16_t port);
		virtual ~TcpListener();

		void init(void);
		void run(void);

	private:
		struct s_buffer
		{
			char *		b;
			streamsize	size;
			streamsize	occupiedSize;
			s_buffer(streamsize s) : b(new char[s]), size(s) {}
			s_buffer(s_buffer const & other) {b = other.b; size = other.size;}
			~s_buffer() {delete[] b;}
			private:
				s_buffer & operator=(s_buffer const &);
		};
		typedef queue<s_buffer *, list<s_buffer *> > t_bufferQ;

		in_addr_t		_ipAddress;
		const uint16_t	_port;
		int				_socket;
		const int		_backlog;
		fd_set			_activeFdSet;
		int				_clientNb;

		TcpListener(void);
		TcpListener(TcpListener const & other);
		TcpListener & operator=(TcpListener const & other);

		void _acceptNewClient(void) throw(tcpException);
		void _disconnectClient(SOCKET client);
		void _handleRequest(SOCKET client) throw(tcpException);
		void _answerToClient(SOCKET client, HttpRequest & request)
			throw(sendException, tcpException);
		string const _getRequiredFile(HttpRequest const & request) const;
		void _sendToClient(SOCKET client, char const * msg, size_t size)
			const throw(sendException);
		void _sendStatus(SOCKET client,
			HttpRequest::s_status const & status)
			const throw(sendException);
		void _sendEndOfHeader(SOCKET client) const throw(sendException);
		void _sendFile(SOCKET client, char const * fileName,
			struct stat const & fileInfos)
			const throw(sendException, tcpException);
		void _sendBody(SOCKET client, t_bufferQ & bufferQ)
			const throw(sendException);
		void _writeServerField(std::ostringstream & headerStream) const;
		void _writeDateField(std::ostringstream & headerStream) const;
		void _writeContentFields(std::ostringstream & headerStream,
			char const * fileName, struct stat const & fileInfos,
			t_bufferQ const & bufferQ) const;
		t_bufferQ _getFile(char const * fileName)
			const throw(tcpException);
};

#endif