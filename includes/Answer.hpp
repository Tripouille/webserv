#ifndef ANSWER_HPP
# define ANSWER_HPP

# include <string>
# include <map>
# include "BufferQ.hpp"
# include "HttpRequest.hpp"

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

		Answer(SOCKET client);
		~Answer();
		Answer(Answer const & other);

		Answer & operator=(Answer const & other);
		void setBody(t_bufferQ const & body);
		void getFile(string const & fileName) throw(sendException);
		void sendStatus(HttpRequest::s_status const & status)
			const throw(sendException);
		void sendHeader(void) const throw(sendException);
		void sendEndOfHeader(void) const throw(sendException);
		void sendAnswer(string const & fileName) throw(sendException);

	private:
		SOCKET						_client;
		std::map<string, string>	_fields;
		t_bufferQ					_body;

		Answer(void);
		void _copy(Answer const & other);
		void _sendToClient(char const * msg, size_t size)
			const throw(sendException);
		void _sendBody(void) throw(sendException);
		void _fillServerField(void);
		void _fillDateField(void);
		void _fillContentFields(string const & fileName);
};

#endif