/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/12/01 15:12:42 by frfrey           ###   ########lyon.fr   */
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
# include <cstring>
# include <cerrno>

using std::string;
using std::vector;
using std::map;
using std::ofstream;
using std::ifstream;

class ServerConfig
{

	public:

		class tcpException : public std::exception
		{
			public:

				tcpException(string str = "") throw();
				virtual ~tcpException(void) throw() {}
				virtual const char * what(void) const throw();

			private:

				string _str;
		};

		explicit ServerConfig( std::string const & path = "server.conf" );
		~ServerConfig();

/*
** --------------------------------- METHODS ----------------------------------
*/
		void									init(void);

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/
		string const 							getUser( void ) const;
		string const 							getWorker( void ) const;
		pid_t		 							getPid( void ) const;
		string const 							getPathModules( void ) const;
		short									getWorkerConnections( void ) const;
		bool									getMultiAccept( void ) const;
		bool									getSendfile( void ) const;
		bool									getTcpNoPush( void ) const;
		bool									getTcpNoDelay( void ) const;
		int										getKeepAliveTimeout( void ) const;
		int										getTypeHashMaxSize( void ) const;
		bool									getServerTokens( void ) const;
		int										getServerNameHashBucketSize( void ) const;
		bool									getServerNameInRedirect( void ) const;
		map<string, string> const &				getMimeType( void ) const;
		string const 							getDefaultType( void ) const;
		bool									getGzip( void ) const;
		bool									getGzipVary( void ) const;
		string const 							getGzipProxied( void ) const;
		int										getGzipCompLevel( void ) const;
		int										getGzipBuffers( void ) const;
		string const 							getGzipVersion( void ) const;
		vector<string> const &					getGzipType( void ) const;
		vector<int> const &						getPortListen( void ) const;
		string const 							getPathRoot( void ) const;
		vector<string> const &					getIndex( void ) const;
		string const 							getServerName( void ) const;
		map<string, vector<string> > const &	getFastcgi( void ) const;

		ofstream &								getPathAccessLog( void ) const;
		ofstream &								getPathErrorLog( void ) const;

	private:

		string									_pathConfFile;
		/* Header file config */
		string									_user;					// Define user when server start
		string									_worker;				// Number max of CPU use
		pid_t									_pid;					// pid of process
		string									_pathModules; 			// path config file module enable;

		/*	Events	*/
		short									_workerConnections;
		bool									_multiAccept;

		/* Http */
		/* Basic Seeting */
		bool									_sendfile;
		bool									_tcpNoPush;
		bool									_tcpNoDelay;
		int										_keepAliveTimeout;		// Define a time period during connection will open
		int										_typeHashMaxSize;		// Define max size of hash tables
		bool									_serverTokens;			// Enable the transmission of your server version
		int										_serverNameHashBucketSize;
		bool									_serverNameInRedirect;
		map<string, string>						_mimeType;				// type file return by server
		string									_defaultType;			// Define the type MIME default

		/* Log Settings*/
		ofstream								_pathAccessLog;			// Who requetes is writting
		ofstream								_pathErrorLog;			// Error log is writting

		/* GZIP Setting */
		bool									_gzip;					// Enable gzip compress
		bool									_gzipVary;
		string									_gzipProxied;
		int										_gzipCompLevel;
		int										_gzipBuffers;
		string									_gzipVersion;
		vector<string>							_gzipType;

		/* Virtual Host Config */
		vector<int>								_portListen;
		string									_pathRoot;
		vector<string>							_index;
		string									_serverName;
		map<string, vector<string> >			_fastcgi;

/*
** ----------------------------- PRIVATE METHODS ------------------------------
*/

		ServerConfig( ServerConfig const & src );
		ServerConfig &		operator=( ServerConfig const & rhs );

};

#endif /* **************************************************** SERVERCONFIG_H */
