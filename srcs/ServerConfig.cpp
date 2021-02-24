/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2021/02/24 23:13:19 by frfrey           ###   ########lyon.fr   */
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
	_dictionary.push_back("port");
	_dictionary.push_back("root");
	_dictionary.push_back("server_name");
	_dictionary.push_back("index");
	_dictionary.push_back("error_page");
	_dictionary.push_back("location");
	_dictionary.push_back("cgi");
	_dictionary.push_back("allowed_methods");
	_dictionary.push_back("upload_store");
	_dictionary.push_back("client_max_body_size");
	_dictionary.push_back("alias");
	_dictionary.push_back("auth_basic");
	_dictionary.push_back("auth_basic_user_file");
	_dictionary.push_back("autoindex");
	_dictionary.push_back("user");
	_dictionary.push_back("worker_processes");
	_dictionary.push_back("pid");
	_dictionary.push_back("worker_connections");
	_dictionary.push_back("uri_max_size");
	_dictionary.push_back("max_empty_line_before_request");
	_dictionary.push_back("sendfile");
	_dictionary.push_back("tcp_nopush");
	_dictionary.push_back("tcp_nodelay");
	_dictionary.push_back("keepalive_timeout");
	_dictionary.push_back("types_hash_max_size");
	_dictionary.push_back("type_file");
	_dictionary.push_back("default_type");
	_dictionary.push_back("access_log");
	_dictionary.push_back("error_log");
	_dictionary.push_back("gzip");
	_dictionary.push_back("host");
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

void					ServerConfig::checkKeyIsNotValid( string const & p_key, int *nbLine )
{
	for (list<string>::iterator it = _dictionary.begin(); \
		it != _dictionary.end(); it++)
		if (*it == p_key)
			return ;
	throw std::invalid_argument("Error: line " + toStr(*nbLine) \
		+ " " + p_key + " is invalid. ");
}

bool					ServerConfig::checkArgAllowdMethods( vector<string> & p_vector )
{
	vector<string>::iterator it = p_vector.begin();
	for (; it != p_vector.end(); it++)
		if (*it != "HEAD" && *it != "GET" && *it != "POST" && *it != "PUT")
			return true;
	return false;
}

DIR *					ServerConfig::directoryPath( void )
{
	if (http.find("host") == http.end())
	{
		errno = EHOSTUNREACH;
		throw configException("Error path \"host\" does not exist on 'conf' file");
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
		throw configException("Error params \"Port:\" does not exist",
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

bool					ServerConfig::checkAutoIndex( map<string, string> & p_map, \
														string const & p_filename )
{
	if (p_map.find("autoindex") != p_map.end())
	{
		string tmp = p_map.at("autoindex");
		if (tmp == "on" || tmp == "off")
			return (tmp == "on") ? true : false;
		else
		{
			errno = EINVAL;
			throw configException("Error params \"autoindex\" value is not valid on",
									p_filename);
		}
	}
	return false;
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
			if (map->first == "cgi")
			{
				if (map->second.size() == 1)
					checkCgi(map->second[0], p_fileName);
				else
				{
					errno = EINVAL;
					throw configException("Error in params \"" + string(map->first) + "\" multi argument is forbiden :", \
											p_fileName);
				}
			}
			if (map->first == "client_max_body_size")
			{
				if (map->second.size() == 1)
					tryParseInt(map->second[0]);
				else
				{
					errno = EINVAL;
					throw configException("Error in params \"" + string(map->first) + "\" multi argument is forbiden :", \
											p_fileName);
				}
			}
			if (map->first == "allowed_methods" && checkArgAllowdMethods(map->second))
			{
				errno = EINVAL;
				throw configException("Error in params \"" + string(map->first) + "\" invalid argument :", \
										p_fileName);
			}
			if ((map->first != "allowed_methods" && map->first != "index" && map->first != "return") \
				&& (map->second.size() > 1))
			{
				errno = EINVAL;
				throw configException("Error in params \"" + string(map->first) + "\" multi argument is forbiden :", \
											p_fileName);
			}
			if ((map->first == "upload_store"))
			{
				struct stat fileInfos;

				if (stat(map->second.at(0).c_str(), &fileInfos) != 0 || !S_ISDIR(fileInfos.st_mode))
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
			if (map->first == "autoindex")
			{
				if (map->second[0] != "on" && map->second[0] != "off")
				{
					errno = EINVAL;
					throw configException("Error in params \"" + it->first + "\" \'autoindex\' value cannot \'on\' or \'off\' in", \
											p_fileName);
				}
			}
		}
	}
	return p_map;
}

map<Regex, map<string, vector<string> > > &
ServerConfig::checkRegex(map<Regex, map<string, vector<string> > > & p_map, string const & p_fileName)
{
	for (map<Regex, map<string, vector<string> > >::iterator it = p_map.begin(); \
			it != p_map.end(); it++)
	{
		for ( map<string, vector<string> >::iterator map = it->second.begin(); \
				map != it->second.end(); map++)
		{
			if (map->first == "cgi")
			{
				if (map->second.size() == 1)
					checkCgi(map->second[0], p_fileName);
				else
				{
					errno = EINVAL;
					throw configException("Error in params \"" + string(map->first) + "\" multi argument is forbiden :", \
											p_fileName);
				}
			}
			if (map->first == "allowed_methods" && checkArgAllowdMethods(map->second))
			{
				errno = EINVAL;
				throw configException("Error in params \"" + string(map->first) + "\" wrong argument", \
										p_fileName);
			}
			if ((map->first != "allowed_methods" && map->first != "index" && map->first != "return") \
				&& (map->second.size() > 1))
			{
				errno = EINVAL;
				throw configException("Error in params \"" + string(map->first) + "\" multi argument is forbiden :", \
											p_fileName);
			}
			if ((map->first == "upload_store"))
			{
				struct stat fileInfos;

				if (stat(map->second.at(0).c_str(), &fileInfos) != 0 || !S_ISDIR(fileInfos.st_mode))
					throw configException("Error in params \"" + map->first + "\" on Location \"" + it->first.getSource() + \
								"\", " + map->second.at(0) + "\" is not a Directory in file", p_fileName);
			}
			if (map->first == "auth_basic" && (it->second.find("auth_basic_user_file") == it->second.end()))
			{
				errno = EINVAL;
				throw configException("Error in params \"" + it->first.getSource() + "\" need params \'auth_basic_user_file\' :", \
											p_fileName);
			}
			else if (map->first == "auth_basic_user_file" && (it->second.find("auth_basic") == it->second.end()))
			{
				errno = EINVAL;
				throw configException("Error in params \"" + it->first.getSource() + "\" need params \'auth_basic\' :", \
											p_fileName);
			}
			if (map->first == "autoindex")
			{
				if (map->second[0] != "on" && map->second[0] != "off")
				{
					errno = EINVAL;
					throw configException("Error in params \"" + it->first.getSource() + "\" \'autoindex\' value cannot \'on\' or \'off\' :", \
											p_fileName);
				}
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
			throw configException("Error with error file " + string(p_root + it->second) + " :", p_fileName);
		}
	}
	return p_map;
}

void		ServerConfig::checkCgi( string const & p_path, string const & p_fileName ) const
{
	struct stat fileInfos;
	if (stat(p_path.c_str(), &fileInfos) != 0)
	{
		throw configException("Error with cgi path " + p_path + " :", p_fileName);
	}
}

map<string, string>
ServerConfig::isErrorPage( ifstream & p_file, int *nbLine )
{
	string		line;
	string		key;
	string		arg;

	map<string, string>		tmp;
	std::map<string, string>::iterator		it = tmp.begin();

	while(getline(p_file, line))
	{
		*nbLine += 1;
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
		else
			throw std::invalid_argument("Error: line " + toStr(*nbLine) + " not finish.");
		if (arg.find_first_not_of(' ') != string::npos)
			arg.erase(0, arg.find_first_not_of(' '));
		tmp.insert(it, std::pair<string, string>(key, arg));
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
			throw configException("Error params \"alias:\" exist in params location :", p_fileName);
		}
	}
	else if (p_key == "alias")
	{
		if (p_map.find("root") != p_map.end())
		{
			errno = EINVAL;
			throw configException("Error params \"root:\" exist in params location :", p_fileName);
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
ServerConfig::isRegex( map<Regex, map<string, vector<string> > > & p_map, ifstream & p_file, \
								string & p_arg, string const & p_fileName, int *nbLine )
{
	size_t		pos(0);
	string		key;
	string		line;
	string		arg;
	string		root(p_arg);
	map<string, vector<string> >					tmp;
	std::map<string, vector<string> >::iterator		it = tmp.begin();

	if (root[0] == '~')
	{
		root.erase(0,1);
		if ((pos = root.find_last_of('{')) != string::npos)
		{
			if (checkEndLine(root.substr(pos, root.size()), "{"))
				throw std::invalid_argument("Error: line " + toStr(*nbLine) + " not finish.");
			root.erase(pos, root.size());
		}
		std::stringstream tmp(root);
		tmp >> line;
	}
	if (line.find_first_of(' ') != string::npos)
		line.erase(line.find_first_of(' '), line.size());
	try {
		Regex regex(line.c_str());
		while(getline(p_file, line))
		{
			*nbLine += 1;
			std::stringstream		str(line);
			str >> key;
			this->checkKeyInvalid(key, tmp, p_fileName);
			if (str.eof() && key != "}" && key != "{")
				this->checkKeyIsNotValid(key, nbLine);
			if (str.eof() && key != "}")
				continue;
			if (key.at(0) == '#')
				continue;
			if (key == "}" && line != "}")
				throw std::invalid_argument("Error: line " + toStr(*nbLine) + " Invalid argument: " + p_fileName);
			if (key == "}")
				break ;
			this->checkKeyIsNotValid(key, nbLine);
			getline(str, arg);
			if ((pos = arg.find_first_of(';')) != string::npos)
			{
				if (checkEndLine(arg.substr(pos, arg.size()), ";"))
					throw std::invalid_argument("Error: line " + toStr(*nbLine) + " not finish.");
				arg.erase(arg.find_first_of(';'), arg.size());
			}
			else
				throw std::invalid_argument("Error: line " + toStr(*nbLine) + " not finish");
			if (arg.find_first_not_of(' ') != string::npos)
				arg.erase(0, arg.find_first_not_of(' '));
			vector<string> tmpV = this->splitArg(arg);
			tmp.insert(it, std::pair<string, vector<string> >(key, tmpV));
		}
		p_map[regex] = tmp;

	}
	catch (std::invalid_argument const & e) {
		errno = EINVAL;
		throw configException("Error: " + string(e.what()) + " :", p_fileName);
	}
}

void
ServerConfig::isLocation( map<string, map<string, vector<string> > > & p_map, ifstream & p_file, \
								string & p_arg, string const & p_fileName, int *nbLine )
{
	size_t		pos(0);
	string		key;
	string		line;
	string		root(p_arg);
	string		arg;
	map<string, vector<string> >					tmp;
	std::map<string, vector<string> >::iterator		it = tmp.begin();

	if ((pos = root.find_last_of('{')) != string::npos)
	{
		if (checkEndLine(root.substr(pos, root.size()), "{"))
				throw std::invalid_argument("Error: line " + toStr(*nbLine) + " not finish");
		root.erase(pos, root.size());
	}
	if (root.find_first_of(' ') != string::npos)
		root.erase(root.find_first_of(' '), root.size());
	while(getline(p_file, line))
	{
		*nbLine += 1;
		std::stringstream		str(line);
		str >> key;
		this->checkKeyInvalid(key, tmp, p_fileName);
		if (str.eof() && key != "}" && key != "{")
				this->checkKeyIsNotValid(key, nbLine);
		if (str.eof() && key != "}")
			continue;
		if (key.at(0) == '#')
			continue;
		if (key == "}" && line != "}")
			throw std::invalid_argument("Error: line " + toStr(*nbLine) + " Invalid argument: " + p_fileName);
		if (key == "}")
			break ;
		this->checkKeyIsNotValid(key, nbLine);
		getline(str, arg);
		if (( pos = arg.find_first_of(';')) != string::npos)
		{
			if (checkEndLine(arg.substr(pos, arg.size()), ";"))
					throw std::invalid_argument("Error: line " + toStr(*nbLine) + " not finish.");
			arg.erase(arg.find_first_of(';'), arg.size());
		}
		else
			throw std::invalid_argument("Error: line " + toStr(*nbLine) + " not finish.");
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
			map<string, string>							tmp;
			map<string, string>							errorTmp;
			std::map<string, string>::iterator			it = tmp.begin();
			map<string, map<string, vector<string> > >	conf;
			map<Regex, map<string, vector<string> > >	regex;
			uint16_t									port(0);
			int											nbLine(0);

			while (getline(hostFile, line))
			{
				nbLine++;
				std::stringstream	str(line);

				str >> key;
				if (str.eof())
					continue;
				if (key.at(0) == '#')
					continue;
				this->checkKeyIsNotValid(key, &nbLine);
				getline(str, arg);
				if (arg.find_first_of(';') != string::npos)
				{
					if (checkEndLine(arg.substr(arg.find_first_of(';'), arg.size()), ";"))
						throw std::invalid_argument("Error: line " + toStr(nbLine) + " not finish.");
					arg.erase(arg.find_first_of(';'), arg.size());
				}
				else if ((key == "port" || key == "root" || key == "server_name" || key == "index") \
					&& arg.find_first_of(';') == string::npos)
					throw std::invalid_argument("Error: line " + toStr(nbLine) + " not finish.");
				if (arg.find_first_not_of(' ') != string::npos)
					arg.erase(0, arg.find_first_not_of(' '));
				if (key == "port")
					port = tryParseInt(arg);
				else if (key == "error_page")
					errorTmp = this->isErrorPage(hostFile, &nbLine);
				else if (key == "location")
				{
					if (arg[0] == '~')
						this->isRegex(regex, hostFile, arg, p_filname[i], &nbLine);
					else
						this->isLocation(conf, hostFile, arg, p_filname[i], &nbLine);
				}
				else
				{
					this->checkKeyExist(key, tmp, p_filname[i]);
					tmp.insert(it, std::pair<string, string>(key, arg));
				}
			}

			for (map<Regex, map<string, vector<string> > >::iterator reg = regex.begin(); reg != regex.end(); reg++)
			{
				std::cout << "DEBUG REG: " << reg->first.getSource() << std::endl << "\t";
				for (map<string, vector<string> >::iterator maps = reg->second.begin(); maps != reg->second.end(); maps++)
				{
					std::cout << maps->first << std::endl << "\t\t";
					for (vector<string>::iterator vec = maps->second.begin(); vec != maps->second.end(); vec++)
						std::cout << *vec << " ";
					std::cout << std::endl;
				}
			}

			for (map<string, map<string, vector<string> > >::iterator reg = conf.begin(); reg != conf.end(); reg++)
			{
				std::cout << "DEBUG LOCATION: " << reg->first << std::endl << "\t";
				for (map<string, vector<string> >::iterator maps = reg->second.begin(); maps != reg->second.end(); maps++)
				{
					std::cout << maps->first << std::endl << "\t\t";
					for (vector<string>::iterator vec = maps->second.begin(); vec != maps->second.end(); vec++)
						std::cout << *vec << " ";
					std::cout << std::endl;
				}
			}

			/* Init stuct Host */
			Host temp_host = {
				this->checkPort(port, p_filname[i]),
				this->checkRoot(tmp, p_filname[i]),
				this->checkAutoIndex(tmp, p_filname[i]),
				this->convertIndex(tmp, p_filname[i]),
				this->checkServerName(tmp, p_filname[i]),
				this->checkErrorPage(errorTmp, p_filname[i], tmp.at("root")),
				this->checkLocation(conf, p_filname[i]),
				this->checkRegex(regex, p_filname[i])
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
		throw configException("Error params type_file does not exist : server.conf");
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
	int					nbLine(0);
	std::map<string, string>::iterator		it = http.begin();

	while (getline(file, line))
	{
		std::stringstream	str(line);

		nbLine++;
		str >> key;
		if (str.eof())
			continue;
		if (key.at(0) == '#')
			continue;
		if ((nb = key.find_first_of(':') != string::npos))
			key.erase(key.find_first_of(':'), nb + 1);
		this->checkKeyIsNotValid(key, &nbLine);
		getline(str, arg);
		if ((nb = arg.find_first_of(';')) != string::npos)
		{
			if (checkEndLine(arg.substr(nb, arg.size()), ";"))
				throw std::invalid_argument("Error: line " + toStr(nbLine) + " not finish.");
			arg.erase(nb, arg.size());
		}
		if ((nb = arg.find_first_of(';')) != string::npos)
			arg.erase(nb, arg.size());
		if ((nb = arg.find_first_not_of(' ')) != string::npos)
			arg.erase(0, nb);
		this->checkKeyExist(key, http);
		http.insert(it, std::pair<string, string>(key, arg));
		arg = "";
		key = "";
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

/*
** -------------------------------- ACCESSEUR ---------------------------------
*/

/* ************************************************************************** */
