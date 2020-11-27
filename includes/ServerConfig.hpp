/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/11/27 12:00:15 by frfrey           ###   ########lyon.fr   */
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

		ServerConfig( std::string const & path = "server.conf" );
		~ServerConfig();

		string			getUser( void ) const;
		string			getWorker( void ) const;
		pid_t			getPid( void ) const;
		string			getPathModules( void ) const;
		short			getWorkerConnections( void ) const;
		bool			getMultiAccept( void ) const;

	private:

		/* Header file config */
		string						_user;					// Define user when server start
		string						_worker;				// Number max of CPU use
		pid_t						_pid;					// pid of process
		string						_pathModules; 			// path config file module enable;

		/*	Events	*/
		short						_workerConnections;
		bool						_multiAccept;

		/* Http */
		/* Basic Seeting */
		bool						_sendfile;
		bool						_tcpNoPush;
		bool						_tcpNoDelay;
		int							_keepAliveTimeout;		// Define a time period during connection will open
		int							_typeHashMaxSize;		// Define max size of hash tables
		bool						_serverTokens;			// Enable the transmission of your server version
		int							_serverNameHashBucketSize;
		bool						_serverNameInRedirect;
		map<string, string>			_mimeType;				// type file return by server
		string						_defaultType;			// Define the type MIME default

		/* Log Settings*/
		ofstream					_access_log;			// Who requetes is writting
		ofstream					_error_log;				// Error log is writting

		/* GZIP Setting */
		bool						_gzip;					// Enable gzip compress
		bool						_gzipVary;
		string						_gzipProxied;
		int							_gzipCompLevel;
		int							_gzipBuffers;
		int							_gzipVersion;
		vector<string>				_gzipType;

		/* Virtual Host Config */
		vector<int>					_portListen;
		string						_pathRoot;
		vector<string>				_index;
		string						_serverName;
		map<int, vector<string> >	_fastcgi;


		ServerConfig( ServerConfig const & src );
		ServerConfig &		operator=( ServerConfig const & rhs );

};

#endif /* **************************************************** SERVERCONFIG_H */
