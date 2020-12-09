/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/12/09 15:19:03 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ServerConfig.hpp"
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <algorithm>


/*
** -------------------------------- Exception ---------------------------------
*/

ServerConfig::tcpException::tcpException(string str) throw()
	: _str(str + " : " + strerror(errno))
{
}

const char *	ServerConfig::tcpException::what(void) const throw()
{
	return (_str.c_str());
}

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

ServerConfig::ServerConfig( std::string const & path ) :
	_pathConfFile(path), _nbLine(0)
{
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

ServerConfig::~ServerConfig()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/


/*
** --------------------------------- METHODS ----------------------------------
*/

void									ServerConfig::readFile( ifstream & file )
{
	string				line;
	string				key;
	string				arg;
	std::map<string, string>::iterator		it = _http.begin();
	ofstream			pid;

	while (getline(file, line))
	{
		std::stringstream	str(line);
		_nbLine++;

		str >> key;
		if (str.eof())
			continue;
		if (key.at(0) == '#' || key.at(0) == '}')
			continue;
		getline(str, arg);
		if (arg.find_first_of(';') != string::npos)
			arg.erase(arg.find_first_of(';'), arg.size());
		if (arg.find_first_not_of(' ') != string::npos)
			arg.erase(0, arg.find_first_not_of(' '));
		_http.insert(it, std::pair<string, string>(key, arg));
	}

	/* Save PID program on file */
	pid.open(_http.at("pid").c_str());
	pid << getpid() << std::endl;
	pid.close();
	for (std::map<string, string>::iterator i = _http.begin(); i != _http.end(); ++i)
	{
		std::cout << i->first << " : ";
		std::cout << i->second << std::endl;
	}
}

void									ServerConfig::init( void )
{
	ifstream configFile(_pathConfFile.c_str());
	if (configFile)
	{
		this->readFile(configFile);
	} else {
		throw tcpException("Error with config file");
	}
}

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/

/* ************************************************************************** */
