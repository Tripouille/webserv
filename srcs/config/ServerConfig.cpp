/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/11/27 16:57:56 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/ServerConfig.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

ServerConfig::ServerConfig( std::string const & path = "server.conf" ) :
	_user(""), _worker(""), _pid(getPid()), _pathModules(""), _workerConnections(0),
	_multiAccept(false), _sendfile(false), _tcpNoPush(false), _tcpNoDelay(false),
	_keepAliveTimeout(0), _typeHashMaxSize(0), _serverTokens(false), _serverNameHashBucketSize(0),
	_serverNameInRedirect(false), _mimeType(map<string, string>()), _defaultType(""),
	_pathAccessLog(""), _pathErrorLog(""), _gzip(false), _gzipVary(false), _gzipProxied(false),
	_gzipCompLevel(0), _gzipBuffers(0), _gzipVersion(""), _gzipType(vector<string>()),
	_portListen(vector<int>(80)), _pathRoot(""), _index(vector<string>()), _serverName(""),
	_fastcgi(map<string, vector<string> >())
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

string const &							ServerConfig::getUser( void ) const { return "www-data"; }
string const &							ServerConfig::getWorker( void ) const { return "auto"; }
pid_t const 							ServerConfig::getPid( void ) const { return getPid();}
string const &							ServerConfig::getPathModules( void ) const { return ""; }
short const								ServerConfig::getWorkerConnections( void ) const { return 1024; }
bool const								ServerConfig::getMultiAccept( void ) const { return false; }
bool const								ServerConfig::getSendfile( void ) const { return true; }
bool const								ServerConfig::getTcpNoPush( void ) const { return true; }
bool const								ServerConfig::getTcpNoDelay( void ) const { return true; }
int const								ServerConfig::getKeepAliveTimeout( void ) const { return 65; }
int const								ServerConfig::getTypeHashMaxSize( void ) const { return 2048; }
bool const								ServerConfig::getServerTokens( void ) const {return false; }
int const								ServerConfig::getServerNameHashBucketSize( void ) const { return 64; }
bool const								ServerConfig::getServerNameInRedirect( void ) const { return false; }
map<string, string> const &				ServerConfig::getMimeType( void ) const { return _mimeType; }
string const &							ServerConfig::getDefaultType( void ) const { return "application/octet-stream"; }
bool const								ServerConfig::getGzip( void ) const { return true; }
bool const								ServerConfig::getGzipVary( void ) const { return false; }
string const &							ServerConfig::getGzipProxied( void ) const { return ""; }
int const								ServerConfig::getGzipCompLevel( void ) const { return 6;}
int const								ServerConfig::getGzipBuffers( void ) const { return 16; }
string const &							ServerConfig::getGzipVersion( void ) const { return "1.1"; }
vector<string> const &					ServerConfig::getGzipType( void ) const { return _gzipType; }
vector<int> const &						ServerConfig::getPortListen( void ) const { return _portListen; }
string const &							ServerConfig::getPathRoot( void ) const { return "www/"; }
vector<string> const &					ServerConfig::getIndex( void ) const { return _index; }
string const &							ServerConfig::getServerName( void ) const { return "_"; }
map<string, vector<string> > const &	ServerConfig::getFastcgi( void ) const { return _fastcgi; }


/* ************************************************************************** */
