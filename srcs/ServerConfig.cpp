/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/12/16 19:11:09 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
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

DIR *									ServerConfig::directoryPath( void )
{
	string name(_http.at("host").c_str());
	name.erase(name.find_last_of('/'), name.size());
	return opendir(name.c_str());
}

void									ServerConfig::initHost( void )
{

	DIR *				dir = NULL;
	struct dirent *		ent = NULL;
	string				line;
	string				key;
	string				arg;
	//size_t				nb(0);
	vector<string>		fileName;

	dir = this->directoryPath();
	if (dir)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
			std::cout << ent->d_name << std::endl;
			fileName.push_back(string(ent->d_name));
			// ifstream  hostFile(ent->d_name);
			// if (hostFile)
			// {
			// 	while (getline(hostFile, line))
			// 	{
			// 		map<string, string>	tmp;
			// 		std::map<string, string>::iterator		it = tmp.begin();
			// 		std::stringstream	str(line);

			// 		str >> key;
			// 		if (str.eof())
			// 			continue;
			// 		if (key.at(0) == '#')
			// 			continue;
			// 		if ((nb = key.find_first_of(':') != string::npos))
			// 			key.erase(key.find_first_of(':'), nb + 1);
			// 		getline(str, arg);
			// 		if (arg.find_first_of(';') != string::npos)
			// 			arg.erase(arg.find_first_of(';'), arg.size());
			// 		if (arg.find_first_not_of(' ') != string::npos)
			// 			arg.erase(0, arg.find_first_not_of(' '));
			// 		tmp.insert(it, std::pair<string, string>(key, arg));

			// 		for (std::map<string, string>::iterator i = tmp.begin(); i != tmp.end(); ++i)
			// 		{
			// 			std::cout << i->first << " : ";
			// 			std::cout << i->second << std::endl;
			// 		}
			// 	}
			// } else {
			// 	throw tcpException("Error with file in folder host");
			// }
		}
		closedir(dir);
	} else {
		throw tcpException("Error with folder host");
	}
	for(size_t i = 0; i < fileName.size(); i++)
		std::cout << "Name File" << i << ": " << fileName[i] << std::endl;
}

void									ServerConfig::initConf( void )
{
	ofstream								pid;
	string									line;
	string									key;
	string									arg;
	ifstream 								mimeFile(_http.at("type_file").c_str());
	std::map<string, string>::iterator		it = _mimeType.begin();

	/* Save PID program on file */
	pid.open(_http.at("pid").c_str());
	pid << getpid() << std::endl;
	pid.close();

	/* Charg mime.type on map */
	if (mimeFile)
	{
		while (getline(mimeFile, line))
		{
			std::stringstream str(line);
			str >> arg;
			if (arg == "types" || arg == "}")
				continue;
			while (!str.eof())
			{
				str >> key;
				if (key.find_first_of(';') != string::npos)
					key.erase(key.find_first_of(';'), key.size());
				_mimeType.insert(it, std::pair<string, string>(key, arg));
			}
		}
	} else {
		throw tcpException("Error with mime.types file");
	}
}

void									ServerConfig::readFile( ifstream & file )
{
	string				line;
	string				key;
	string				arg;
	size_t				nb(0);
	std::map<string, string>::iterator		it = _http.begin();

	while (getline(file, line))
	{
		std::stringstream	str(line);
		_nbLine++;

		str >> key;
		if (str.eof())
			continue;
		if (key.at(0) == '#')
			continue;
		if ((nb = key.find_first_of(':') != string::npos))
			key.erase(key.find_first_of(':'), nb + 1);
		getline(str, arg);
		if (arg.find_first_of(';') != string::npos)
			arg.erase(arg.find_first_of(';'), arg.size());
		if (arg.find_first_not_of(' ') != string::npos)
			arg.erase(0, arg.find_first_not_of(' '));
		_http.insert(it, std::pair<string, string>(key, arg));
	}

	// for (std::map<string, string>::iterator i = _http.begin(); i != _http.end(); ++i)
	// {
	// 	std::cout << i->first << " : ";
	// 	std::cout << i->second << std::endl;
	// }
}

void									ServerConfig::init( void )
{
	ifstream configFile(_pathConfFile.c_str());
	if (configFile)
	{
		this->readFile(configFile);
		this->initConf();
		this->initHost();
	} else {
		throw tcpException("Error with config file");
	}
}

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/

/* ************************************************************************** */
