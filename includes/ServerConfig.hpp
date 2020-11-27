/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/11/27 16:41:32 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <string>
# include <iostream>
# include <fstream>
# include <vector>
# include <map>
# include <sys/types.h>

using std::string;
using std::vector;
using std::map;
using std::ofstream;

class ServerConfig
{

	public:

		explicit ServerConfig( std::string const & path = "server.conf" );
		~ServerConfig();


		/* Accesseur */
		string const &							getUser( void ) const;
		string const &							getWorker( void ) const;
		pid_t const 							getPid( void ) const;
		string const &							getPathModules( void ) const;
		short const								getWorkerConnections( void ) const;
		bool const								getMultiAccept( void ) const;
		bool const								getSendfile( void ) const;
		bool const								getTcpNoPush( void ) const;
		bool const								getTcpNoDelay( void ) const;
		int const								getKeepAliveTimeout( void ) const;
		int const								getTypeHashMaxSize( void ) const;
		bool const								getServerTokens( void ) const;
		int const								getServerNameHashBucketSize( void ) const;
		bool const								getServerNameInRedirect( void ) const;
		map<string, string> const &				getMimeType( void ) const;
		string const &							getDefaultType( void ) const;
		bool const								getGzip( void ) const;
		bool const								getGzipVary( void ) const;
		string const &							getGzipProxied( void ) const;
		int const								getGzipCompLevel( void ) const;
		int const								getGzipBuffers( void ) const;
		string const &							getGzipVersion( void ) const;
		vector<string> const &					getGzipType( void ) const;
		vector<int> const &						getPortListen( void ) const;
		string const &							getPathRoot( void ) const;
		vector<string> const &					getIndex( void ) const;
		string const &							getServerName( void ) const;
		map<string, vector<string> > const &	getFastcgi( void ) const;

		ofstream &								getPathAccessLog( void ) const;
		ofstream &								getPathErrorLog( void ) const;

	private:

		/* Header file config */
		string							_user;					// Define user when server start
		string							_worker;				// Number max of CPU use
		pid_t							_pid;					// pid of process
		string							_pathModules; 			// path config file module enable;

		/*	Events	*/
		short							_workerConnections;
		bool							_multiAccept;

		/* Http */
		/* Basic Seeting */
		bool							_sendfile;
		bool							_tcpNoPush;
		bool							_tcpNoDelay;
		int								_keepAliveTimeout;		// Define a time period during connection will open
		int								_typeHashMaxSize;		// Define max size of hash tables
		bool							_serverTokens;			// Enable the transmission of your server version
		int								_serverNameHashBucketSize;
		bool							_serverNameInRedirect;
		map<string, string>				_mimeType;				// type file return by server
		string							_defaultType;			// Define the type MIME default

		/* Log Settings*/
		ofstream						_pathAccessLog;			// Who requetes is writting
		ofstream						_pathErrorLog;			// Error log is writting

		/* GZIP Setting */
		bool							_gzip;					// Enable gzip compress
		bool							_gzipVary;
		string							_gzipProxied;
		int								_gzipCompLevel;
		int								_gzipBuffers;
		string							_gzipVersion;
		vector<string>					_gzipType;

		/* Virtual Host Config */
		vector<int>						_portListen;
		string							_pathRoot;
		vector<string>					_index;
		string							_serverName;
		map<string, vector<string> >	_fastcgi;


		ServerConfig( ServerConfig const & src );
		ServerConfig &		operator=( ServerConfig const & rhs );

};

#endif /* **************************************************** SERVERCONFIG_H */
