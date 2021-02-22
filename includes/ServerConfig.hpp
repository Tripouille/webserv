/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2021/02/22 14:20:38 by frfrey           ###   ########lyon.fr   */
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
# include <sys/stat.h>
# include <stdint.h>
# include "Regex.hpp"

using std::string;
using std::list;
using std::vector;
using std::map;
using std::ofstream;
using std::ifstream;

struct Host
{
	uint16_t									port;
	string										root;
	bool										autoIndex;
	vector<string>								index;
	string										serverName;
	map<string, string>							errorPage;
	map<string, map<string, vector<string> > > 	location;
	map<Regex, map<string, vector<string> > >	regexLocation;
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
		vector<string>			convertIndex( map<string, string> & p_map, string const & p_fileName );
		string					checkRoot( map<string, string> & p_map, string const & p_fileName );
		bool					checkAutoIndex( map<string, string> & p_map, string const & p_filename );
		string					checkServerName( map<string, string> & p_map, string const & p_fileName );
		uint16_t &				checkPort( uint16_t & p_port, string const & p_fileName );
		void					checkCgi( string const & p_map, string const & p_fileName ) const;
		map<string, string> &	checkErrorPage ( map<string, string> & p_map, string const & p_fileName, \
													string const & p_root );
		map<string, map<string, vector<string> > > &
								checkLocation( map<string, map<string, vector<string> > > & p_map, string const & p_fileName );
		map<Regex, map<string, vector<string> > > &
								checkRegex( map<Regex, map<string, vector<string> > > & p_map, string const & p_fileName);
		void					checkKeyExist( string const & p_key, map<string, string> const & p_tmp,
												 string const & p_filename = "server.conf" );
		void					checkIfParamsExist( void );
		void					checkKeyInvalid( string const & p_key, map<string, vector<string> > & p_map, \
													string const & p_fileName );
		map<string, string>		isErrorPage( ifstream & p_file );
		void					isLocation(	map<string, map<string, vector<string> > > & p_map, ifstream & p_file, \
												string & p_arg, string const & p_fileName );
		void					isRegex( map<Regex, map<string, vector<string> > > & p_map, ifstream & p_file, \
												string & p_arg, string const & p_fileName );
		vector<string>			splitArg( string & p_arg );

};

#endif /* **************************************************** SERVERCONFIG_H */
