/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2021/01/26 14:41:52 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <string>
# include <iostream>
# include <fstream>
# include <vector>
# include <map>
# include <list>
# include <sys/types.h>
# include <cstring>
# include <cerrno>
# include <dirent.h>
# include <sstream>

using std::string;
using std::list;
using std::vector;
using std::map;
using std::ofstream;
using std::ifstream;

struct Host
{
	uint16_t							port;
	string								root;
	vector<string>						index;
	string								serverName;
	map<string, map<string, string> > 	conf;
};

class ServerConfig
{

	public:

		class configException : public std::exception
		{
			public:

				configException(string str = "", string arg = "") throw();
				virtual ~configException(void) throw() {}
				virtual const char * what(void) const throw();

			private:

				string _str;
		};

		explicit ServerConfig( std::string const & path = "conf/server.conf" );
		~ServerConfig();

/*
** --------------------------------- METHODS ----------------------------------
*/
		void									init( void );
		void									checkConfigFile( void );

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/



		map<string, string>						http;
		map<string, string>						mimeType;
		vector<Host>							host;

	private:

		string									_pathConfFile;

/*
** ----------------------------- PRIVATE METHODS ------------------------------
*/

		ServerConfig( ServerConfig const & src );
		ServerConfig &			operator=( ServerConfig const & rhs );
		void					readFile( ifstream & file );
		void					initConf( void );
		void					readFolderHost( void );
		void					initHost( vector<string> & p_filname );
		DIR *					directoryPath( void );
		vector<string>			convertIndex( map<string, string> & p_map, string & p_fileName );
		string					checkRoot( map<string, string> & p_map, string & p_fileName );
		string					checkServerName( map<string, string> & p_map, string & p_fileName );
		uint16_t &				checkPort( uint16_t & p_port, string & p_fileName );
		map<string, map<string, string> > &
								checkConf( map<string, map<string, string> > & p_map, string & p_fileName );
		void					checkKeyExist( string const & p_key, map<string, string> const & p_tmp,
												 string const & p_filename = "server.conf" );
		void					checkIfParamsExist( void );
		map<string, string>		isCgi( string const & p_key, ifstream & p_file );
		map<string, string>		isErrorPage( string const & p_key, string & p_arg, \
												ifstream & p_file, string const & p_root );
		void					isLocation(	map<string, map<string, string> > & p_map, \
											ifstream & p_file, string & p_arg, string const & p_root );

};

#endif /* **************************************************** SERVERCONFIG_H */
