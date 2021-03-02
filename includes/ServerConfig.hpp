/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2021/03/02 15:00:39 by frfrey           ###   ########lyon.fr   */
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
# include "utils.hpp"

# define WHITESPACE " \n\t\r\v\f"
# define DEFAULT_WORKER_PROCESSES "8"
# define DEFAULT_URI_MAX_SIZE "512"
# define DEFAULT_MAX_EMPTY_LINE_BEFORE_REQUEST "1"

using std::string;
using std::list;
using std::vector;
using std::map;
using std::ofstream;
using std::ifstream;

struct Host
{
	short										port;
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

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/



		map<string, string>						http;
		map<string, string>						mimeType;
		vector<Host>							host;

	private:

		string									_pathConfFile;
		list<string>							_fileConf;
		list<string>							_fileHost;
		list<string>							_location;

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
		vector<string>			convertIndex( map<string, string> & p_map );
		string					checkRoot( map<string, string> & p_map, string const & p_fileName );
		bool					checkAutoIndex( map<string, string> & p_map, string const & p_filename );
		string					checkServerName( map<string, string> & p_map, string const & p_fileName );
		short &					checkPort( short & p_port, string const & p_fileName );
		void					checkCgi( string const & p_map, string const & p_fileName ) const;
		map<string, string> &	checkErrorPage ( map<string, string> & p_map, string const & p_fileName, \
													string const & p_root );
		map<string, map<string, vector<string> > > &
								checkLocation( map<string, map<string, vector<string> > > & p_map, string const & p_fileName );
		map<Regex, map<string, vector<string> > > &
								checkRegex( map<Regex, map<string, vector<string> > > & p_map, string const & p_fileName);
		void					checkKeyExist( string const & p_key, map<string, string> const & p_tmp,
												 string const & p_filename = "server.conf" );
		void					checkKeyExist( string const & p_key, map<string, vector<string> > const & p_tmp,
												 string const & p_filename = "server.conf" );
		void					checkKeyExist( string const & p_key, map<string, map<string, vector<string> > > const & p_tmp,
												 string const & p_filename = "server.conf" );
		void					checkKeyExist( Regex const & p_key, map<Regex, map<string, vector<string> > > & p_tmp,
												 string const & p_filename = "server.conf" );
		void					checkIfParamsExist( void );
		void					checkIfKeyIsNotRootOrAlias( string const & p_key, map<string, vector<string> > & p_map, \
													string const & p_fileName );
		void					isErrorPage( ifstream & p_file, int *nbLine, string const & p_fileName, \
											string & p_arg, map<string, string> & p_mapError );
		void					isLocation(	map<string, map<string, vector<string> > > & p_map, ifstream & p_file, \
												string & p_arg, string const & p_fileName, int *nbLine );
		void					isRegex( map<Regex, map<string, vector<string> > > & p_map, ifstream & p_file, \
												string & p_arg, string const & p_fileName, int *nbLine );
		vector<string>			splitArg( std::stringstream & p_sstr, bool & p_bracketIsClose, \
										int * nbLine, string const & p_fileName );
		void					checkKeyIsNotValid( string const & p_key, int *nbLine, list<string> & p_list );
		bool					checkArgAllowdMethods( vector<string> & p_vector );
		void					fillLocation( string const & p_fileName, ifstream & p_file,\
										map<string, vector<string> > & p_location,  int * nbLine, \
										bool p_bracketIsOpen, string & p_line );
		string					checkOpeningBracket( string & p_root, bool & bracketIsOpen );
		void					checkErrorCode( string & p_key, int *nbLine, string const & p_fileName );
		string					extractBraquetErrorPage( string & p_arg, int *nbLine );
		bool					checkArgumentSolo( string & p_arg );
		bool					checkArgumentErrorPage( string & p_arg );
		bool					checkArgumentLocation( string & p_arg );
		string 					checkBracketLine(string & p_key, std::stringstream & p_str, char c);
		void					clearBuff(std::stringstream & p_str);
		void					checkEndLineFileLocation(string & p_line, int * nbLine, string const & p_fileName);

};

#endif /* **************************************************** SERVERCONFIG_H */
