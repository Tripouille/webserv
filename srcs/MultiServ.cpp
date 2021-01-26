/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultiServ.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/19 10:39:00 by frfrey            #+#    #+#             */
/*   Updated: 2021/01/25 15:28:44 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "MultiServ.hpp"

/*
** -------------------------------- Exception ---------------------------------
*/

MultiServ::servException::servException( string str, string arg ) throw()
	: _str(str + " " + arg + " : " + strerror(errno))
{

}

const char *			MultiServ::servException::what(void) const throw()
{
	return (_str.c_str());
}

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

MultiServ::MultiServ( ServerConfig & p_config, vector<Host> & p_host )
	: _config(p_config), _host(p_host)
{
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

MultiServ::~MultiServ()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

void			MultiServ::initServs( void )
{
	if (_config.http.find("pid") != _config.http.end())
		_pids.open(_config.http.at("pid").c_str());
	if (_pids)
	{
		for (std::vector<Host>::iterator host = _host.begin(); host != _host.end(); host++)
		{
			pid_t pid;
			if ((pid = fork()) < 0)
			{
					std::cerr << "Error: fork failled" << std::endl;
					exit(10);
			}
			if (pid == 0)
			{
				_pids << getpid() << std::endl;
				TcpListener webserv(INADDR_ANY, host->port, _config, *host);
				webserv.init();
				webserv.run();
			}
			else
			{
			}
		}
		wait(&_status);
		_pids.close();
	}
	else
	{
		errno = 2;
		throw servException("Error file pid is not open:");
	}
}

void			MultiServ::stopServ( char * p_arg )
{
	if (!(strcmp("stop", p_arg)))
	{
		ifstream	file;
		string		arg;
		string		fileName;

		if (_config.http.find("pid") != _config.http.end())
		{
			fileName = _config.http.at("pid");
			file.open(fileName.c_str());
			if (file)
			{
				std::cerr << "Extinction des serveurs !" << std::endl;
				while (getline(file, arg))
				{
					pid_t tmp;

					tmp = atoi(arg.c_str());
					kill(tmp, 15);
				}
			}
			else
			{
				errno = 2;
				throw servException("Error file is not open for stop serv:", fileName);
			}
		} else {
			errno = 2;
			throw servException("Error file pid not exist:");
		}
		file.close();
	}
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/


/* ************************************************************************** */
