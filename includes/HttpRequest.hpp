#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP
# include <string>
# include <map>
# include <cstdio>

using std::map;
using std::string;

typedef int SOCKET;

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
		ssize_t _getLine(char * buffer);

		SOCKET 					_client;
		string 					_method, _target, _httpVersion;
		map<string, string>		_fields;
		string					_body;
		s_status				_status;
};

#endif