/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/12/09 15:07:45 by frfrey           ###   ########lyon.fr   */
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

	private:

		string									_pathConfFile;
		map<string, string>						_http;
		map<string, string>						_Server;
		int										_nbLine;

/*
** ----------------------------- PRIVATE METHODS ------------------------------
*/

		ServerConfig( ServerConfig const & src );
		ServerConfig &		operator=( ServerConfig const & rhs );
		void				readFile( ifstream & file );

};

#endif /* **************************************************** SERVERCONFIG_H */
