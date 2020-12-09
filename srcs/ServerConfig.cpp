/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/12/09 12:59:31 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerConfig.hpp"
#include <sstream>
#include <unistd.h>
#include <sys/types.h>


/*
** -------------------------------- Exception ---------------------------------
*/

ServerConfig::tcpException::tcpException(string str) throw()
	: _str(str + " : " + strerror(errno))
{
}

const char *	ServerConfig::tcpException::what(void) const throw()
{
	return (_str.c_str());
}

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

ServerConfig::ServerConfig( std::string const & path ) :
	// _pathConfFile(path), _user(""), _worker(""), _pid(getPid()), _pathModules(""),
	// _workerConnections(0), _multiAccept(false), _sendfile(false), _tcpNoPush(false),
	// _tcpNoDelay(false), _keepAliveTimeout(0), _typeHashMaxSize(0), _serverTokens(false),
	// _serverNameHashBucketSize(0), _serverNameInRedirect(false),
	// _mimeType(map<string, string>()), _defaultType(""), _pathAccessLog(""),
	// _pathErrorLog(""), _gzip(false), _gzipVary(false), _gzipProxied(""), _gzipCompLevel(0),
	// _gzipBuffers(0), _gzipVersion(""), _gzipType(vector<string>()),
	// _portListen(vector<int>(80)), _pathRoot(""), _index(vector<string>()), _serverName("_"),
	// _fastcgi(map<string, vector<string> >())
	_pathConfFile(path), _nbLine(0)
{
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

ServerConfig::~ServerConfig()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/


/*
** --------------------------------- METHODS ----------------------------------
*/

void									ServerConfig::readFile( ifstream & file )
{
	string				line;
	string				key;
	string				arg;
	std::map<string, string>::iterator		it = _http.begin();

	while (getline(file, line))
	{
		std::stringstream	str(line);
		_nbLine++;

		str >> key;
		if (str.eof())
			continue;
		if (key.at(0) == '#' || key.at(0) == '}')
			continue;
		getline(str, arg);
		arg.find_first_of(';');
		_http.insert(it, std::pair<string, string>(key, arg));
		//std::cout << "Line: " << _nbLine << " " << key << " : " << arg << std::endl;
	}

	for (std::map<string, string>::iterator i = _http.begin(); i != _http.end(); ++i)
	{
		std::cout << i->first << " : ";
		std::cout << i->second << std::endl;
	}
}

void									ServerConfig::init( void )
{
	ifstream configFile(_pathConfFile.c_str());
	if (configFile)
	{
		this->readFile(configFile);
	} else {
		throw tcpException("Error with config file");
	}
}

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/

// string const 							ServerConfig::getUser( void ) const { return string("www-data"); }
// string const 							ServerConfig::getWorker( void ) const { return string("auto"); }
// pid_t		 							ServerConfig::getPid( void ) const { return _pid; }
// string const 							ServerConfig::getPathModules( void ) const { return ""; }
// short									ServerConfig::getWorkerConnections( void ) const { return 1024; }
// bool									ServerConfig::getMultiAccept( void ) const { return false; }
// bool									ServerConfig::getSendfile( void ) const { return true; }
// bool									ServerConfig::getTcpNoPush( void ) const { return true; }
// bool									ServerConfig::getTcpNoDelay( void ) const { return true; }
// int										ServerConfig::getKeepAliveTimeout( void ) const { return 65; }
// int										ServerConfig::getTypeHashMaxSize( void ) const { return 2048; }
// bool									ServerConfig::getServerTokens( void ) const {return false; }
// int										ServerConfig::getServerNameHashBucketSize( void ) const { return 64; }
// bool									ServerConfig::getServerNameInRedirect( void ) const { return false; }
// map<string, string> const &				ServerConfig::getMimeType( void ) const { return _mimeType; }
// string const 							ServerConfig::getDefaultType( void ) const { return string("application/octet-stream"); }
// bool									ServerConfig::getGzip( void ) const { return true; }
// bool									ServerConfig::getGzipVary( void ) const { return false; }
// string const 							ServerConfig::getGzipProxied( void ) const { return ""; }
// int										ServerConfig::getGzipCompLevel( void ) const { return 6;}
// int										ServerConfig::getGzipBuffers( void ) const { return 16; }
// string const 							ServerConfig::getGzipVersion( void ) const { return string("1.1"); }
// vector<string> const &					ServerConfig::getGzipType( void ) const { return _gzipType; }
// vector<int> const &						ServerConfig::getPortListen( void ) const { return _portListen; }
// string const 							ServerConfig::getPathRoot( void ) const { return string("www/"); }
// vector<string> const &					ServerConfig::getIndex( void ) const { return _index; }
// string const 							ServerConfig::getServerName( void ) const { return string("_"); }
// map<string, vector<string> > const &	ServerConfig::getFastcgi( void ) const { return _fastcgi; }


/* ************************************************************************** */
