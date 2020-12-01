#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP
# include <string>
# include <map>
# include <sys/types.h>
# include <iostream>

# define CLIENT_MAX_BODY_SIZE 1000000
# define REQUEST_LINE_MAX_SIZE 1024
# define HEADER_MAX_SIZE 8000

typedef int SOCKET;

using std::map;
using std::string;

class HttpRequest
{
	public:
		/* Sets status on throw */
		class parseException : public std::exception
		{
			public:
				parseException(HttpRequest & request, string str, int code, string const & info) throw();
				virtual ~parseException(void) throw();
				virtual const char * what(void) const throw();
			private:
				string _str;
		};
		class closeOrderException : public std::exception
		{
			public:
				closeOrderException(void) throw();
				virtual ~closeOrderException(void) throw();
				virtual const char * what(void) const throw();
		};

		HttpRequest(SOCKET client);
		~HttpRequest(void);
		HttpRequest(HttpRequest const & other);

		HttpRequest & operator=(HttpRequest const & other);

		void setStatus(int c, string const & i);

		void analyze(void) throw(parseException);

	private:
		HttpRequest(void);
		struct s_status
		{
			int		code;
			string	info;
		};

		void _copy(HttpRequest const & other);
		void _analyseRequestLine(void) throw(parseException);
		ssize_t _getLine(char * buffer, size_t limit) const;

		SOCKET 					_client;
		string 					_method, _target, _httpVersion;
		map<string, string>		_fields;
		string					_body;
		s_status				_status;
};

#endif