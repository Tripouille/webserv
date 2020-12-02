#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP
# include <string>
# include <map>
# include <vector>
# include <sys/types.h>
# include <iostream>

# define CLIENT_MAX_BODY_SIZE 1000000
# define REQUEST_LINE_MAX_SIZE 1024
# define HEADER_MAX_SIZE 8000

typedef int SOCKET;

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::map;
using std::vector;

class HttpRequest
{
	public:
		struct s_status
		{
			int		code;
			string	info;
		};
		
		/* Sets status on throw */
		class parseException : public std::exception
		{
			public:
				parseException(HttpRequest const & request, string str, int code, string const & info) throw();
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

		s_status const & getStatus(void) const;
		void setStatus(int c, string const & i);

		void analyze(void) throw(parseException, closeOrderException);

	private:
		HttpRequest(void);

		void _copy(HttpRequest const & other);
		void _analyseRequestLine(void) throw(parseException, closeOrderException);
		ssize_t _getLine(char * buffer, ssize_t limit) const throw(parseException);
		vector<string> _split(string s, char delim) const;
		void _checkMethod(void) const throw(parseException);
		void _checkTarget(void) const throw(parseException);
		void _checkHttpVersion(void) const throw(parseException);

		SOCKET 					_client;
		string 					_method, _target, _httpVersion;
		map<string, string>		_fields;
		string					_body;
		s_status				_status;
};

#endif