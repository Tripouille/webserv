/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2021/01/18 15:03:33 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include <sstream>
#include <unistd.h>
#include <algorithm>


/*
** -------------------------------- Exception ---------------------------------
*/

ServerConfig::configException::configException(string str, string arg) throw()
	: _str(str + " " + arg + " : " + strerror(errno))
{
}

const char *			ServerConfig::configException::what(void) const throw()
{
	return (_str.c_str());
}

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

ServerConfig::ServerConfig( std::string const & path ) :
	_pathConfFile(path)
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

DIR *					ServerConfig::directoryPath( void )
{
	if (http.find("host") == http.end())
	{
		errno = 113;
		throw configException("Error path \"host\" does not exist on conf file");
	}
	string name(http.at("host").c_str());
	name.erase(name.find_last_of('/'), name.size());
	return opendir(name.c_str());
}

void					ServerConfig::checkKeyExist( string const & p_key,
														map<string, string> const & p_tmp,
														string const & p_filename )
{
	if (p_tmp.find(p_key.c_str())!= p_tmp.end())
	{
		string error("Error params \"");
		error += p_key;
		error += "\" already exist in file \"";
		error += p_filename;
		error += "\".";
		errno = 22;
		throw configException(error);
	}
}

map<string, string> &	ServerConfig::checkCgi( map<string, string> & p_map )
{
	return p_map;
}

vector<uint16_t> &			ServerConfig::checkPort( vector<uint16_t> & p_vector,
													string & p_fileName )
{
	if (p_vector.empty())
	{
		errno = 22;
		throw configException("Error params \"server_name:\" not found on ",
								p_fileName);
	}
	return p_vector;
}

string					ServerConfig::checkServerName( map<string, string> & p_map,
															string & p_fileName )
{
	if (p_map.find("server_name") == p_map.end())
	{
		errno = 22;
		throw configException("Error params \"server_name:\" not found on ",
								p_fileName);
	}
	return string(p_map.at("server_name"));
}

string					ServerConfig::checkRoot( map<string, string> & p_map,
														string & p_fileName )
{
	if (p_map.find("root") == p_map.end())
	{
		errno = 22;
		throw configException("Error params \"root:\" not found on ",
								p_fileName);
	}
	return string(p_map.at("root"));
}

vector<string>			ServerConfig::convertIndex( map<string, string> & p_map,
														string & p_fileName )
{
	std::string			word;
	vector<string>		tmp;

	if (p_map.find("index") == p_map.end())
	{
		errno = 22;
		throw configException("Error params \"index:\" not found in file ",
								p_fileName);
	}
	std::stringstream	line(p_map.at("index"));
	while(line)
	{
		line >> word;
		tmp.push_back(word);
	}
	return tmp;
}

void					ServerConfig::initHost( vector<string> & p_filname )
{
	string				line;
	string				key;
	string				arg;
	size_t				nb(0);

	for (size_t i = 0; i < p_filname.size(); i++)
	{
		string fileName(http.at("host"));
		fileName += p_filname[i];
		ifstream  hostFile(fileName.c_str());
		if (hostFile)
		{
			map<string, string>	tmp;
			std::map<string, string>::iterator		it = tmp.begin();
			vector<uint16_t> port;
			map<string, string> cgi;
			std::map<string, string>::iterator		it2 = cgi.begin();

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
					port.push_back(static_cast<unsigned short>(atoi(arg.c_str())));
				else if (!strncmp(string(key).c_str(), "cgi", 3))
				{
					str >> key;
					cgi.insert(it2, std::pair<string, string>(key, arg));
				}
				else
				{
					this->checkKeyExist(key, tmp, p_filname[i]);
					tmp.insert(it, std::pair<string, string>(key, arg));
				}
			}
			Host temp_host = {
				this->checkPort(port, p_filname[i]),
				this->checkRoot(tmp, p_filname[i]),
				this->convertIndex(tmp, p_filname[i]),
				this->checkServerName(tmp, p_filname[i]),
				this->checkCgi(cgi)
			};
			host.push_back(temp_host);
		} else {
			throw configException("Error with file", p_filname[i]);
		}
	}
}

void					ServerConfig::readFolderHost( void )
{

	DIR *				dir = NULL;
	struct dirent *		ent = NULL;
	vector<string>		fileName;

	dir = this->directoryPath();
	if (dir)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_type == DT_DIR)
				continue ;
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
			fileName.push_back(string(ent->d_name));
		}
		closedir(dir);
	} else {
		throw configException("Error with folder ", http.at("host"));
	}
	this->initHost(fileName);
}

void					ServerConfig::initConf( void )
{
	ofstream			pid;
	string				line;
	string				key;
	string				arg;

	std::map<string, string>::iterator		it = mimeType.begin();

	/* Check if params Type_file exist */
	if (http.find("type_file") == http.end())
	{
		errno = 22;
		throw configException("Error params type_file does not exist on conf file");
	}
	ifstream 			mimeFile(http.at("type_file").c_str());

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
				mimeType.insert(it, std::pair<string, string>(key, arg));
			}
		}
		mimeFile.close();
	} else {
		throw configException("Error with path ", http.at("type_file"));
	}
}

void					ServerConfig::readFile( ifstream & file )
{
	string				line;
	string				key;
	string				arg;
	size_t				nb(0);
	std::map<string, string>::iterator		it = http.begin();

	while (getline(file, line))
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
		this->checkKeyExist(key, http);
		http.insert(it, std::pair<string, string>(key, arg));
	}

	// for (std::map<string, string>::iterator i = _http.begin(); i != _http.end(); ++i)
	// {
	// 	std::cout << i->first << " : ";
	// 	std::cout << i->second << std::endl;
	// }
}

void					ServerConfig::init( void )
{
	ifstream configFile(_pathConfFile.c_str());
	if (configFile)
	{
		this->readFile(configFile);
		this->initConf();
		this->readFolderHost();
		configFile.close();
		this->checkIfParamsExist();
	} else {
		throw configException("Error with config file", _pathConfFile);
	}
}

void					ServerConfig::checkIfParamsExist( void )
{
	if (http.find("uri_max_size") == http.end())
	{
		errno = 22;
		throw configException("Error in config file with params uri_max_size not exist");
	}
	if (http.find("max_empty_line_before_request") == http.end())
	{
		errno = 22;
		throw configException("Error in config file with params max_empty_line_before_request not exist");
	}
}

void					ServerConfig::checkConfigFile( void )
{
	ifstream	configFile(_pathConfFile.c_str());
	string		line;
	string		params;

	if (configFile)
	{
		while(getline(configFile, line))
		{
			std::stringstream	str(line);
			str >> params;
			if (str.eof())
				continue;
			if (params.at(0) == '#')
				continue;
			if (line.find_last_of(';') == string::npos)
			{
				errno = 22;
				throw configException("Error in config file with params :", params);
			}
		}
		configFile.close();
	}
}

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/

/* ************************************************************************** */
