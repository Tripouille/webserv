#ifndef ANSWER_HPP
# define ANSWER_HPP

# include <string>
# include <map>
# include <errno.h>
# include "BufferQ.hpp"
# include "HttpRequest.hpp"
# include "ServerConfig.hpp"

# define HTTP_VERSION "HTTP/1.1"
# define BUFFER_SIZE 1024

typedef int SOCKET;
using std::string;

class Answer
{
	friend class TcpListener;
	friend class CgiRequest;

	public:
		class sendException : public std::exception
		{
			public:
				sendException(string str = "") throw();
				virtual ~sendException(void) throw();
				virtual const char * what(void) const throw();
			private:
				string _str;
		};

		Answer(SOCKET client, Host const & host, ServerConfig const & config);
		~Answer();
		Answer(Answer const & other);

		Answer & operator=(Answer const & other);

		void getFile(string const & fileName) throw(sendException);
		void setErrorFields(t_bufferQ const & body);
		void sendStatus(HttpRequest::s_status const & status)
			const throw(sendException);
		void sendHeader(void) const throw(sendException);
		void sendEndOfHeader(void) const throw(sendException);
		void sendAnswer(HttpRequest const & request) throw(sendException);

	private:
		SOCKET						_client;
		std::map<string, string>	_fields;
		t_bufferQ					_body;
		Host const &				_host;
		ServerConfig const &		_config;

		Answer(void);
		void _copy(Answer const & other);
		void _sendToClient(char const * msg, size_t size)
			const throw(sendException);
		void _sendBody(void) throw(sendException);
		void _fillServerField(void);
		void _fillDateField(void);
		void _fillContentFields(HttpRequest const & request);
		void _debugFields(void);
};

#endif