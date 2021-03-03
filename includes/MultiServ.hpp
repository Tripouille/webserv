/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultiServ.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/19 10:39:00 by frfrey            #+#    #+#             */
/*   Updated: 2021/03/03 14:07:16 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTISERV_HPP
# define MULTISERV_HPP

# include <iostream>
# include <string>
# include <unistd.h>
# include <netinet/in.h>
# include <csignal>

# include "ServerConfig.hpp"
# include "TcpListener.hpp"

class MultiServ
{

	public:

		class servException : public std::exception
		{
			public:

				servException(string str = "", string arg = "") throw();
				virtual ~servException(void) throw() {}
				virtual const char * what(void) const throw();

			private:

				string _str;
		};

		MultiServ( ServerConfig & p_config, vector<Host> & p_host );
		~MultiServ();

		void						initServs( void );
		void						stopServ( char * p_str );
		static void					stopServSignal( int sig );

	private:

		ServerConfig &				_config;
		vector<Host> &				_host;
		ofstream					_pids;
		int							_status;
		static MultiServ * 			instance;

		MultiServ &		operator=( MultiServ const & rhs );
		MultiServ( MultiServ const & src );
		void						eraseFile();

};

#endif /* ******************************************************* MULTISERV_H */
