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
		class parseException : public std::exception
		{
			public:
				parseException(string str = "") throw();
				virtual ~parseException(void) throw();
				virtual const char * what(void) const throw();
			private:
				string _str;
		};

		HttpRequest(SOCKET client);
		~HttpRequest(void);
		HttpRequest(HttpRequest const & other);

		HttpRequest & operator=(HttpRequest const & other);

		void analyze(void) throw(parseException);

	private:
		HttpRequest(void);
		struct s_status
		{
			int		code;
			string	info;
		};

		void _copy(HttpRequest const & other);
		void _setStatus(int c, string const & i);
		ssize_t _getLine(char * buffer, size_t limit) const;

		SOCKET 					_client;
		string 					_method, _target, _httpVersion;
		map<string, string>		_fields;
		string					_body;
		s_status				_status;
};

#endif