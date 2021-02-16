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
# include <fstream>
# include <md5.hpp>
# include <algorithm>
# include "Client.hpp"
# include "ServerConfig.hpp"
# include "base64.hpp"
# include "utils.hpp"

# define CLIENT_MAX_BODY_SIZE 100000000
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
	friend class TcpListener;
	friend class CgiRequest;
	friend class Answer;
	public:
		struct s_status
		{
			int		code;
			string	info;
		};

		struct realmInfos
		{
			string	name;
			string	userFile;
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
		class directoryListingException : public std::exception
		{
			public:
				directoryListingException(void) throw();
				virtual ~directoryListingException(void) throw();
				virtual const char * what(void) const throw();
		};
		class closeOrderException : public std::exception
		{
			public:
				closeOrderException(void) throw();
				virtual ~closeOrderException(void) throw();
				virtual const char * what(void) const throw();
		};

		HttpRequest(Client & client, Host& host, ServerConfig & config);
		~HttpRequest(void);
		HttpRequest(HttpRequest const & other);

		s_status const & getStatus(void) const;
		void setStatus(int c, string const & i);

		void analyze(void) throw(parseException, closeOrderException, directoryListingException);

	private:
		Client &						_client;
		string 							_method, _target, _httpVersion;
		string							_requiredFile, _fileWithoutRoot;
		string							_queryPart, _extensionPart;
		bool							_fileFound;
		realmInfos						_requiredRealm;
		map<string, vector<string> >	_fields;
		char *							_body;
		size_t							_bodySize;
		ssize_t							_headerSize;
		s_status						_status;
		Host &							_host;
		ServerConfig &					_config;
		std::map<string, std::pair<string, string> > _realms;

		HttpRequest(void);
		HttpRequest & operator=(HttpRequest const & other);

		void _copy(HttpRequest const & other);
		void _analyseRequestLine(void) throw(parseException, closeOrderException);
		ssize_t _getLine(char * buffer, ssize_t limit, bool LFAllowed = true)
		const throw(parseException);
		vector<string> _splitRequestLine(string s) const;
		void _fillAndCheckRequestLine(vector<string> const & requestLine)
		throw(parseException);
		void _checkMethod(void) const throw(parseException);
		void _checkTarget(void) const throw(parseException);
		void _checkHttpVersion(void) const throw(parseException);
		void _analyseHeader(void) throw(parseException);
		void _parseHeaderLine(string line) throw(parseException);
		void _splitHeaderField(string s, vector<string> & fieldValue) const;
		void _checkHeader(void) throw(parseException);
		void _analyseBody(void) throw(parseException);
		void _analyseNormalBody(int maxBodySize) throw(parseException);
		void _analyseChunkedBody(int maxBodySize) throw(parseException);
		void _checkContentLength(vector<string> const & contentLengthField)
		const throw(parseException);
		map<string, vector<string> > _getDeepestLocation(string const & key, string & analyzedFile) const;
		void _setRequiredFile(void);
		void _extractQueryPart(void);
		string _getPath(string file) const;
		void _addIndexIfDirectory(void);
		bool _searchForIndexInLocations(void);
		bool _searchForIndexInHost(void);
		bool _directoryListingIsActivated(void) const;
		string _getLanguageAndEncodingExtension(void);
		vector<vector<string> > _getVariantFilesInDirectory(void);
		vector<std::pair<string, double> > _getAcceptedExtensions(string const & fieldKey);
		bool _updateStatusIfInvalid(void);
		void _useStoreDirectory(void);
		bool _methodIsAuthorized(void) const;
		void _setRequiredRealm(void);
		void _setClientInfos(void) const;
		bool _isAuthorized(void) const;
		void _debugFields(void);
};

#endif
