/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2020/12/17 17:39:22 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include <sstream>
#include <unistd.h>
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

vector<string>							ServerConfig::convertIndex( map<string, string> & p_map )
{
	std::string			word;
	vector<string>		tmp;

	if (p_map.find("index") == p_map.end())
	{
		errno = 22;
		throw tcpException("Error index not found in server conf");
	}
	std::stringstream	line(p_map.at("index"));
	while(line)
	{
		line >> word;
		tmp.push_back(word);
	}
	return tmp;
}

void									ServerConfig::initHost( vector<string> & p_filname )
{
	string				line;
	string				key;
	string				arg;
	size_t				nb(0);

	for (size_t i = 0; i < p_filname.size(); i++)
	{
		string fileName(_http.at("host"));
		fileName += p_filname[i];
		ifstream  hostFile(fileName.c_str());
		if (hostFile)
		{
			map<string, string>	tmp;
			std::map<string, string>::iterator		it = tmp.begin();
			vector<int> port;
			while (getline(hostFile, line))
			{
				std::stringstream	str(line);

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
				if (key == "port")
					port.push_back(atoi(arg.c_str()));
				else
					tmp.insert(it, std::pair<string, string>(key, arg));
			}
			Host temp_host = {
				port,
				string(tmp.at("root")),
				this->convertIndex(tmp),
				string(tmp.at("server_name")),
				vector<string>()
			};
			_host.push_back(temp_host);
		} else {
			throw tcpException("Error with file in folder host");
		}
	}
}

void									ServerConfig::readFolderHost( void )
{

	DIR *				dir = NULL;
	struct dirent *		ent = NULL;
	vector<string>		fileName;

	dir = this->directoryPath();
	if (dir)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
			fileName.push_back(string(ent->d_name));
		}
		closedir(dir);
	} else {
		throw tcpException("Error with folder host");
	}
	this->initHost(fileName);
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
		this->readFolderHost();
	} else {
		throw tcpException("Error with config file");
	}
}

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/

/* ************************************************************************** */
