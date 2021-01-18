#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP
# include <string>
# include <string.h>
# include <map>
# include <vector>
# include <sys/types.h>
# include <iostream>
# include <sstream>
# include <sys/stat.h>
# include "Client.hpp"
# include "ServerConfig.hpp"

# define CLIENT_MAX_BODY_SIZE 1000000
# define REQUEST_LINE_MAX_SIZE 1024
# define URI_MAX_SIZE 512
# define HEADER_MAX_SIZE 8000
# define MAX_EMPTY_LINE_BEFORE_REQUEST 1

typedef int SOCKET;

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::map;
using std::vector;

class HttpRequest
{
	friend class TcpListener;
	friend class CgiRequest;
	public:
		struct s_status //déplacer dans private puisque friend ?
		{
			int		code;
			string	info;
		};

		/* Sets status on throw */
		class parseException : public std::exception
		{
			public:
				parseException(HttpRequest const & request, int code, string const & info, string str) throw();
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

		HttpRequest(Client & client, Host& host, uint16_t port, ServerConfig & config);
		~HttpRequest(void);
		HttpRequest(HttpRequest const & other);

		s_status const & getStatus(void) const;
		void setStatus(int c, string const & i);

		void analyze(void) throw(parseException, closeOrderException);

	private:
		Client &						_client;
		string 							_method, _target, _httpVersion;
		string							_requiredFile, _queryPart;
		map<string, vector<string> >	_fields;
		char							_body[CLIENT_MAX_BODY_SIZE + 1];
		size_t							_bodySize;
		s_status						_status;
		Host &							_host;
		uint16_t						_port;
		ServerConfig &					_config;

		HttpRequest(void);
		HttpRequest & operator=(HttpRequest const & other);

		void _copy(HttpRequest const & other);
		void _analyseRequestLine(ssize_t & headerSize) throw(parseException, closeOrderException);
		ssize_t _getLine(char * buffer, ssize_t limit) const throw(parseException);
		vector<string> _splitRequestLine(string s) const;
		void _fillAndCheckRequestLine(vector<string> const & requestLine) throw(parseException);
		void _checkMethod(void) const throw(parseException);
		void _checkTarget(void) const throw(parseException);
		void _checkHttpVersion(void) const throw(parseException);
		void _analyseHeader(ssize_t & headerSize) throw(parseException);
		void _parseHeaderLine(string line) throw(parseException);
		void _splitHeaderField(string s, vector<string> & fieldValue) const;
		void _checkHeader(void) throw(parseException);
		void _analyseBody(void) throw(parseException);
		void _checkContentLength(vector<string> const & contentLengthField) const throw(parseException);
		void _setRequiredFile(void);
		void _setClientInfos(void) const;
};

#endif
