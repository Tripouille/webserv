/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/12/18 10:27:42 by frfrey           ###   ########lyon.fr   */
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
# include <dirent.h>

using std::string;
using std::vector;
using std::map;
using std::ofstream;
using std::ifstream;

struct Host
{
	vector<int>			port;
	string				root;
	vector<string>		index;
	string				serverName;
	vector<string>		cgi;
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

		explicit ServerConfig( std::string const & path = "server.conf" );
		~ServerConfig();

/*
** --------------------------------- METHODS ----------------------------------
*/
		void									init(void);
		void									chargeHost( ifstream & p_file );

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/



		map<string, string>						_http;
		map<string, string>						_mimeType;
		vector<Host>							_host;

	private:

		string									_pathConfFile;
		int										_nbLine;

/*
** ----------------------------- PRIVATE METHODS ------------------------------
*/

		ServerConfig( ServerConfig const & src );
		ServerConfig &		operator=( ServerConfig const & rhs );
		void				readFile( ifstream & file );
		void				initConf( void );
		void				readFolderHost( void );
		void				initHost( vector<string> & p_filname );
		DIR *				directoryPath( void );
		vector<string>		convertIndex( map<string, string> & p_map, string & p_fileName );
		string				checkRoot( map<string, string> & p_map, string & p_fileName );
		string				checkServerName( map<string, string> & p_map, string & p_fileName );
		vector<int> &		checkPort( vector<int> & p_vector, string & p_fileName );

};

#endif /* **************************************************** SERVERCONFIG_H */
