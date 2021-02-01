/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2021/02/01 14:32:30 by frfrey           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
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
		errno = EHOSTUNREACH;
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
		errno = EINVAL;
		throw configException(error);
	}
}

uint16_t &				ServerConfig::checkPort( uint16_t & p_port,
													string const & p_fileName )
{
	if (p_port == 0)
	{
		errno = EINVAL;
		throw configException("Error params \"server_name:\" not found on ",
								p_fileName);
	}
	return p_port;
}

string					ServerConfig::checkServerName( map<string, string> & p_map,
															string const & p_fileName )
{
	if (p_map.find("server_name") == p_map.end())
	{
		errno = EINVAL;
		throw configException("Error params \"server_name:\" not found on ",
								p_fileName);
	}
	return string(p_map.at("server_name"));
}

string					ServerConfig::checkRoot( map<string, string> & p_map,
														string const & p_fileName )
{
	if (p_map.find("root") == p_map.end())
	{
		errno = EINVAL;
		throw configException("Error params \"root:\" not found on ",
								p_fileName);
	}
	return string(p_map.at("root"));
}

vector<string>			ServerConfig::convertIndex( map<string, string> & p_map,
														string const & p_fileName )
{
	std::string			word;
	vector<string>		tmp;

	if (p_map.find("index") == p_map.end())
	{
		errno = EINVAL;
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

map<string, map<string, vector<string> > > &
ServerConfig::checkLocation( map<string, map<string, vector<string> > > & p_map, \
								string const & p_fileName)
{
	for (map<string, map<string, vector<string> > >::iterator it = p_map.begin(); \
			it != p_map.end(); it++)
	{
		for ( map<string, vector<string> >::iterator map = it->second.begin(); \
				map != it->second.end(); map++)
		{
			if ((map->first != "allowed_methods" && map->first != "index" && map->first != "return") \
				&& (map->second.size() > 1))
			{
				errno = EINVAL;
				throw configException("Error in params \"" + string(map->first) + "\" multi argument is forbiden in", \
											p_fileName);
			}
			if ((map->first == "upload_store"))
			{
				struct stat fileInfos;

				if (stat(map->second.at(0).c_str(), &fileInfos) == 0 && S_ISDIR(fileInfos.st_mode))
					;
				else
					throw configException("Error in params \"" + map->first + "\" on Location \"" + it->first + \
								"\", " + map->second.at(0) + "\" is not a Directory in file", p_fileName);
			}
			if (map->first == "auth_basic" && (it->second.find("auth_basic_user_file") == it->second.end()))
			{
				errno = EINVAL;
				throw configException("Error in params \"" + it->first + "\" need params \'auth_basic_user_file\' in", \
											p_fileName);
			}
			else if (map->first == "auth_basic_user_file" && (it->second.find("auth_basic") == it->second.end()))
			{
				errno = EINVAL;
				throw configException("Error in params \"" + it->first + "\" need params \'auth_basic\' in", \
											p_fileName);
			}
		}
	}
	return p_map;
}

map<string, string> &
ServerConfig::checkErrorPage( map<string, string> & p_map, string const & p_fileName, \
								string const & p_root )
{
	for (map<string, string>::iterator it = p_map.begin(); \
			it != p_map.end(); it++)
	{
		struct stat fileInfos;
		if (stat(string(p_root + it->second).c_str(), &fileInfos) != 0)
		{
			throw configException("Error with error file " + string(p_root + it->second) + " in", p_fileName);
		}
	}
	return p_map;
}

map<string, string> &	ServerConfig::checkCgi( map<string, string>& p_map, string const & p_fileName )
{
	for (map<string, string>::iterator it = p_map.begin(); \
			it != p_map.end(); it++)
	{
		struct stat fileInfos;
		if (stat(it->second.c_str(), &fileInfos) != 0)
		{
			throw configException("Error with cgi path " + string(it->second) + " in", p_fileName);
		}
	}
	return p_map;
}

map<string, string>		ServerConfig::isCgi( ifstream & p_file )
{
	string		line;
	string		key;
	string		arg;
	map<string, string>		tmp;
	std::map<string, string>::iterator		it = tmp.begin();

	while(getline(p_file, line))
	{
		std::stringstream	str(line);

		str >> key;
		if (str.eof() && key != "}")
			continue;
		if (key.at(0) == '#')
			continue;
		if (key == "}")
			break ;
		getline(str, arg);
		if (arg.find_first_of(';') != string::npos)
			arg.erase(arg.find_first_of(';'), arg.size());
		if (arg.find_first_not_of(' ') != string::npos)
			arg.erase(0, arg.find_first_not_of(' '));

		tmp.insert(it, std::pair<string, string>(key, arg));
	}
	return tmp;
}

map<string, string>
ServerConfig::isErrorPage( string & p_arg, ifstream & p_file )
{
	string		line;
	string		key;
	string		arg;
	string		root(p_arg);
	map<string, string>		tmp;
	std::map<string, string>::iterator		it = tmp.begin();

	if (root.find_first_of('/') != string::npos)
		root.erase(root.find_first_of('/'), root.size());
	while(getline(p_file, line))
	{
		std::stringstream		str(line);
		str >> key;
		if (str.eof() && key != "}")
			continue;
		if (key.at(0) == '#')
			continue;
		if (key == "}")
			break ;
		getline(str, arg);
		if (arg.find_first_of(';') != string::npos)
			arg.erase(arg.find_first_of(';'), arg.size());
		if (arg.find_first_not_of(' ') != string::npos)
			arg.erase(0, arg.find_first_not_of(' '));
		root += arg;
		tmp.insert(it, std::pair<string, string>(key, root));
	}
	return tmp;
}

void
ServerConfig::checkKeyInvalid( string const & p_key, map<string, vector<string> > & p_map, \
									string const & p_fileName )
{
	if (p_key == "root")
	{
		if (p_map.find("alias") != p_map.end())
		{
			errno = EINVAL;
			throw configException("Error params \"alias:\" exist in params location on file ", p_fileName);
		}
	}
	else if (p_key == "alias")
	{
		if (p_map.find("root") != p_map.end())
		{
			errno = EINVAL;
			throw configException("Error params \"root:\" exist in params location on file ", p_fileName);
		}
	}
}

vector<string>			ServerConfig::splitArg( string & p_arg )
{
	vector<string>		tmp;
	std::stringstream	str(p_arg);
	string				arg;

	while(!str.eof())
	{
		str >> arg;
		tmp.push_back(arg);
	}
	return tmp;
}

void
ServerConfig::isLocation( map<string, map<string, vector<string> > > & p_map, ifstream & p_file, \
								string & p_arg, string const & p_fileName )
{
	string		key;
	string		line;
	string		root(p_arg);
	string		arg;
	map<string, vector<string> >					tmp;
	vector<string>									argV;
	std::map<string, vector<string> >::iterator		it = tmp.begin();

	if (root.find_first_of(' ') != string::npos)
		root.erase(root.find_first_of(' '), root.size());
	while(getline(p_file, line))
	{
		std::stringstream		str(line);
		str >> key;
		this->checkKeyInvalid(key, tmp, p_fileName);
		if (str.eof() && key != "}")
			continue;
		if (key.at(0) == '#')
			continue;
		if (key == "}")
			break ;
		getline(str, arg);
		if (arg.find_first_of(';') != string::npos)
			arg.erase(arg.find_first_of(';'), arg.size());
		if (arg.find_first_not_of(' ') != string::npos)
			arg.erase(0, arg.find_first_not_of(' '));
		vector<string> tmpV = this->splitArg(arg);
		tmp.insert(it, std::pair<string, vector<string> >(key, tmpV));
	}
	p_map[root] = tmp;
}

void					ServerConfig::initHost( vector<string> & p_filname )
{
	string				line;
	string				key;
	string				arg;

	for (size_t i = 0; i < p_filname.size(); i++)
	{
		string fileName(http.at("host"));
		fileName += p_filname[i];
		ifstream  hostFile(fileName.c_str());
		if (hostFile)
		{
			map<string, string>	tmp;
			map<string, string>	errorTmp;
			map<string, string> cgiTmp;
			std::map<string, string>::iterator		it = tmp.begin();
			map<string, map<string, vector<string> > >	conf;
			std::map<string, map<string, vector<string> > >::iterator	it2 = conf.begin();
			uint16_t port;

			while (getline(hostFile, line))
			{
				std::stringstream	str(line);

				str >> key;
				if (str.eof())
					continue;
				if (key.at(0) == '#')
					continue;
				getline(str, arg);
				if (arg.find_first_of(';') != string::npos)
					arg.erase(arg.find_first_of(';'), arg.size());
				if (arg.find_first_not_of(' ') != string::npos)
					arg.erase(0, arg.find_first_not_of(' '));
				if (key == "port")
					port = static_cast<unsigned short>(atoi(arg.c_str()));
				else if (key == "cgi")
					cgiTmp = this->isCgi(hostFile);
				else if (key == "error_page")
					errorTmp = this->isErrorPage(arg, hostFile);
				else if (key == "location")
					this->isLocation(conf, hostFile, arg, p_filname[i]);
				else
				{
					this->checkKeyExist(key, tmp, p_filname[i]);
					tmp.insert(it, std::pair<string, string>(key, arg));
				}
			}
			/* Init stuct Host */
			Host temp_host = {
				this->checkPort(port, p_filname[i]),
				this->checkRoot(tmp, p_filname[i]),
				this->convertIndex(tmp, p_filname[i]),
				this->checkServerName(tmp, p_filname[i]),
				this->checkCgi(cgiTmp, p_filname[i]),
				this->checkErrorPage(errorTmp, p_filname[i], tmp.at("root")),
				this->checkLocation(conf, p_filname[i])
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
		errno = EINVAL;
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
		errno = EINVAL;
		throw configException("Error in config file with params uri_max_size not exist");
	}
	if (http.find("max_empty_line_before_request") == http.end())
	{
		errno = EINVAL;
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
				errno = EINVAL;
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
