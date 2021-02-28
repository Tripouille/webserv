/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: frfrey <frfrey@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/27 10:12:28 by frfrey            #+#    #+#             */
/*   Updated: 2021/02/28 20:48:09 by frfrey           ###   ########lyon.fr   */
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
	/* Charge FileConf word allowed */
	_fileConf.push_back("type_file");
	_fileConf.push_back("worker_processes");
	_fileConf.push_back("pid");
	_fileConf.push_back("uri_max_size");
	_fileConf.push_back("max_empty_line_before_request");
	_fileConf.push_back("host");
	/* Charge FileHost word allowed */
	_fileHost.push_back("port");
	_fileHost.push_back("root");
	_fileHost.push_back("server_name");
	_fileHost.push_back("index");
	_fileHost.push_back("error_page");
	_fileHost.push_back("location");
	_fileHost.push_back("autoindex");
	/* Charge Location word allowed */
	_location.push_back("root");
	_location.push_back("index");
	_location.push_back("cgi");
	_location.push_back("allowed_methods");
	_location.push_back("upload_store");
	_location.push_back("client_max_body_size");
	_location.push_back("alias");
	_location.push_back("auth_basic");
	_location.push_back("auth_basic_user_file");
	_location.push_back("autoindex");
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

ServerConfig::~ServerConfig( )
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/


/*
** --------------------------------- METHODS ----------------------------------
*/

bool					ServerConfig::checkArgumentErrorPage( string & p_arg )
{
	size_t				pos(0);
	string				newArg;
	short				nb(0);

	if ((pos = p_arg.find_first_of('#')) != string::npos)
		newArg = p_arg.substr(0, pos);
	else
		newArg = p_arg;
	std::stringstream	arg(newArg);
	while (!arg.eof())
	{
		string				word;

		arg >> word;
		if (word.empty() || word.find_first_not_of("{") != string::npos)
			break ;
		nb++;
	}
	if (nb > 1)
			return false;
	else if (nb == 0)
		return false;
	return true;
}

bool					ServerConfig::checkArgumentLocation( string & p_arg )
{
	size_t				pos(0);
	string				newArg;
	short				nb(0);

	if ((pos = p_arg.find_first_of('{')) != string::npos)
		newArg = p_arg.substr(0, pos);
	else
		newArg = p_arg;
	std::stringstream	arg(newArg);
	while (!arg.eof())
	{
		string				word;

		arg >> word;
		if (word.empty())
			break ;
		nb++;
	}
	if (nb > 1)
			return false;
	else if (nb == 0)
		return false;
	return true;
}

bool					ServerConfig::checkArgumentSolo( string & p_arg )
{
	std::stringstream	arg(p_arg);
	string				word;
	short				nb(0);

	while (!arg.eof())
	{
		arg >> word;
		if (word[0] == '#')
			break ;
		nb++;
	}
	if (nb > 1)
			return false;
	else if (nb == 0)
		return false;
	return true;
}

void					ServerConfig::checkKeyIsNotValid( string const & p_key, int *nbLine, list<string> & p_list )
{
	for (list<string>::iterator it = p_list.begin(); \
		it != p_list.end(); it++)
		if (*it == p_key)
			return ;
	throw std::invalid_argument("Error: line " + toStr(*nbLine) \
		+ " '" + p_key + "' is invalid. ");
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
	else if (!checkArgumentSolo(p_map.at("server_name")))
			throw std::invalid_argument("Error: Invalid argument: One argument is allowed: " \
						+ p_fileName);
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

vector<string>			ServerConfig::convertIndex( map<string, string> & p_map )
{
	vector<string>		tmp;

	if (p_map.find("index") != p_map.end())
	{
		std::string			word;
		std::stringstream	line(p_map.at("index"));

		while(line)
		{
			line >> word;
			tmp.push_back(word);
		}
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
	if (!checkArgumentSolo(const_cast<string&>(p_root)))
			throw std::invalid_argument("Error: Invalid argument: One argument is allowed: " \
						+ p_fileName);
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

void
ServerConfig::checkIfKeyIsNotRootOrAlias( string const & p_key, map<string, vector<string> > & p_map, \
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

vector<string>			ServerConfig::splitArg( std::stringstream & p_sstr, bool & p_bracketIsClose, \
							int * nbLine, string const & p_fileName )
{
	vector<string>		tmp;
	string				line;
	size_t				pos;

	while(!p_sstr.eof())
	{
		string	arg;

		p_sstr >> line;
		if ((pos = line.find_first_of("{")) != string::npos)
			throw std::invalid_argument("Error: line " + toStr(*nbLine) + \
					" Invalid argument: Bracket was open: " + p_fileName);
		if ((pos = line.find_first_of(';')) != string::npos)
		{
			arg = line.substr(0, pos);
			p_sstr.seekg((line.size() - arg.size() - 1) * -1, std::ios_base::cur);
			if (line[pos + 1] == '}' && checkEndLine(line.substr(pos + 1), "}"))
			{
				p_bracketIsClose = true;
			}
			tmp.push_back(arg);
			break ;
		}
		else
			arg = line;
		tmp.push_back(arg);
	}
	return tmp;
}

void					ServerConfig::fillLocation( string const & p_fileName, ifstream & p_file,\
							map<string, vector<string> > & p_location,  int * nbLine, bool p_bracketIsOpen,
							string & p_line )
{
	string		arg;
	bool		bracketIsClose(false);

	while(!bracketIsClose)
	{
		if (p_line.empty())
		{
			*nbLine += 1;
			getline(p_file, p_line);
		}
		std::stringstream		str(p_line);
		p_line.erase();

		while (!str.eof() && !bracketIsClose)
		{
			string		key;

			str >> key;
			if (key.empty() || key.at(0) == '#')
				break ;
			if (key != "{" && !p_bracketIsOpen)
				throw std::invalid_argument("Error: line " + toStr(*nbLine) + \
					" Invalid argument: Bracket was not open: " + p_fileName);
			if (key == "{")
			{
				if (p_bracketIsOpen)
					throw std::invalid_argument("Error: line " + toStr(*nbLine) + \
						" Invalid argument: Bracket is already open: " + p_fileName);
				p_bracketIsOpen = true;
				key.erase();
				str >> key;
				if (key.empty())
					break ;
			}
			if (key == "}")
			{
				string rest;
				getline(str, rest);
				if (!checkEndLine(rest, "}"))
					throw std::invalid_argument("Error: line " + toStr(*nbLine) + " Invalid argument: " + p_fileName);
				bracketIsClose = true;
				break;
			}
			if (str.eof())
				throw std::invalid_argument("Error: line " + toStr(*nbLine) \
					+ " Invalid argument: Missing value " + p_fileName);
			this->checkIfKeyIsNotRootOrAlias(key, p_location, p_fileName);
			this->checkKeyIsNotValid(key, nbLine, _location);
			vector<string> tmpV = this->splitArg(str, bracketIsClose, nbLine, p_fileName);
			p_location[key] = tmpV;
		}
	}
}

void
ServerConfig::checkErrorCode( string & p_key , int *nbLine, string const & p_fileName )
{
	uint16_t nb = tryParseInt(p_key);
	if (nb < 100 || nb > 999)
		throw std::invalid_argument("Error: line " + toStr(*nbLine) \
					+ " Invalid argument: This is not an valid Error code " \
					+ p_fileName);
}

string
ServerConfig::checkOpeningBracket( string & p_root, bool & bracketIsOpen )
{
	size_t		pos(0);
	string 		key;

	if ((pos = p_root.find_first_not_of(WHITESPACE)) != string::npos)
		p_root.erase(0, pos);
	if ((pos = p_root.find_last_of('{')) != string::npos)
	{
		bracketIsOpen = true;
		key =  p_root.substr(0, pos);
		p_root.erase(0, pos + 1);
	}
	else
	{
		key = p_root;
		p_root.erase();
	}
	return key;
}

void
ServerConfig::isErrorPage( ifstream & p_file, int *nbLine, string const & p_fileName, \
							string & p_arg, map<string, string> & p_mapError )
{
	string					arg;
	bool					bracketIsClose(false);
	bool					bracketIsOpen(false);

	if (!this->checkArgumentErrorPage(p_arg))
		throw std::invalid_argument("Error: line " + toStr(*nbLine) + \
					" Invalid argument: Argument is forbiden: " + p_fileName);
	this->checkOpeningBracket(p_arg, bracketIsOpen);
	while(!bracketIsClose)
	{
		if (p_arg.empty())
		{
			getline(p_file, p_arg);
			*nbLine += 1;
		}
		std::stringstream		str(p_arg);
		p_arg.erase();

		while (!str.eof() && !bracketIsClose)
		{
			string		key;

			str >> key;
			if (key.empty() || key.at(0) == '#')
				break ;
			if (key != "{" && !bracketIsOpen)
				throw std::invalid_argument("Error: line " + toStr(*nbLine) + \
					" Invalid argument: Bracket was not open: " + p_fileName);
			else if (key == "{")
			{
				if (bracketIsOpen)
					throw std::invalid_argument("Error: line " + toStr(*nbLine) + \
						" Invalid argument: Bracket is already open: " + p_fileName);
				bracketIsOpen = true;
				key.erase();
				str >> key;
				if (key.empty())
					break ;
			}
			else if (key == "}")
			{
				string rest;
				getline(str, rest);
				if (!checkEndLine(rest, "}"))
					throw std::invalid_argument("Error: line " + toStr(*nbLine) + " Invalid argument: " + p_fileName);
				bracketIsClose = true;
				break;
			}
			if (str.eof())
				throw std::invalid_argument("Error: line " + toStr(*nbLine) \
					+ " Invalid argument: Missing value " + p_fileName);
			this->checkErrorCode(key, nbLine, p_fileName);
			vector<string> tmpV = this->splitArg(str, bracketIsClose, nbLine, p_fileName);
			if (tmpV.size() == 1)
				p_mapError[key] = tmpV[0];
			else
				throw std::invalid_argument("Error: line " + toStr(*nbLine) \
					+ " Invalid argument: multi argument: " + p_fileName);
		}
	}
}

void
ServerConfig::isRegex( map<Regex, map<string, vector<string> > > & p_map, ifstream & p_file, \
								string & p_arg, string const & p_fileName, int *nbLine )
{
	string							line(p_arg);
	map<string, vector<string> >	location;
	bool							bracketIsOpen(false);
	string							key;
	size_t							pos(0);

	line.erase(0,1);
	key = this->checkOpeningBracket(line, bracketIsOpen);
	if ((pos = key.find_first_of(WHITESPACE)) != string::npos)
			key.erase(0, pos);
	try {
		Regex	regex(key);
		this->fillLocation(p_fileName, p_file, location, nbLine, bracketIsOpen, line);
		p_map[regex] = location;
	}
	catch (std::invalid_argument const & e) {
		errno = EINVAL;
		throw configException("Error: line "+ toStr(*nbLine) + " " + string(e.what()) + " :", p_fileName);
	}
}

void
ServerConfig::isLocation( map<string, map<string, vector<string> > > & p_map, ifstream & p_file, \
								string & p_arg, string const & p_fileName, int *nbLine )
{
	string							line(p_arg);
	map<string, vector<string> >	location;
	bool							bracketIsOpen(false);
	string							key;

	if (!this->checkArgumentLocation(line))
		throw std::invalid_argument("Error: line " + toStr(*nbLine) + \
					" Invalid argument: Argument is forbiden: " + p_fileName);
	key = this->checkOpeningBracket(line, bracketIsOpen);
	this->fillLocation(p_fileName, p_file, location, nbLine, bracketIsOpen, line);
	p_map[key] = location;
}

string					ServerConfig::extractBraquetErrorPage( string & p_arg, int *nbLine )
{
	size_t		pos(0);

	if ((pos = p_arg.find_first_not_of('#')) != string::npos)
	{
		if (p_arg[pos - 1] == '#')
			return string("{");
		throw std::invalid_argument("Error: line " + toStr(*nbLine) + " not finish.");
	}
	return string("{");
}

void					ServerConfig::initHost( vector<string> & p_filname )
{
	string				line;

	for (size_t i = 0; i < p_filname.size(); i++)
	{
		string fileName(http.at("host"));
		fileName += p_filname[i];
		ifstream  hostFile(fileName.c_str());

		if (hostFile)
		{
			map<string, string>							tmp;
			map<string, string>							mapError;
			std::map<string, string>::iterator			it = tmp.begin();
			map<string, map<string, vector<string> > >	conf;
			map<Regex, map<string, vector<string> > >	regex;
			uint16_t									port(0);
			int											nbLine(0);
			size_t										pos(0);

			while (getline(hostFile, line))
			{
				string				key;
				string				arg;
				std::stringstream	str(line);

				nbLine++;
				str >> key;
				if (key.empty())
					continue;
				if (key.at(0) == '#')
					continue;
				getline(str, arg);
				if ((pos = arg.find_first_not_of(WHITESPACE)) != string::npos)
					arg.erase(0, pos);
				if (!std::strncmp(key.c_str(), "error_page", 10))
				{
					if ((pos = key.find_first_of('{')) != string::npos)
						arg = this->extractBraquetErrorPage(arg, &nbLine);
					this->isErrorPage(hostFile, &nbLine, p_filname[i], arg, mapError);
				}
				else if (key == "location")
				{
					if (arg[0] == '~')
						this->isRegex(regex, hostFile, arg, p_filname[i], &nbLine);
					else
						this->isLocation(conf, hostFile, arg, p_filname[i], &nbLine);
				}
				else
				{
					this->checkKeyIsNotValid(key, &nbLine, _fileHost);
					if (arg.find_first_of(';') != string::npos)
					{
						if (!checkEndLine(arg.substr(arg.find_first_of(';'), arg.size()), ";"))
							throw std::invalid_argument("Error: line " + toStr(nbLine) + " not finish.");
						arg.erase(arg.find_first_of(';'), arg.size());
					}
					else
						throw std::invalid_argument("Error: line " + toStr(nbLine) + " not finish.");
					this->checkKeyExist(key, tmp, p_filname[i]);
					if (key == "port")
					{
						if (port != 0)
							throw std::invalid_argument("Error: line " + toStr(nbLine) + " key Port exist!");
						port = tryParseInt(arg);
					}
					else
						tmp.insert(it, std::pair<string, string>(key, arg));
				}
			}

			for (map<string, string>::iterator t = tmp.begin(); t != tmp.end(); t++)
			{
				std::cout << "DEBUG REST: " << std::endl << "\t";
				std::cout << t->first << " " << t->second << std::endl;
			}
			// for (map<Regex, map<string, vector<string> > >::iterator reg = regex.begin(); reg != regex.end(); reg++)
			// {
			// 	std::cout << "DEBUG REG: " << reg->first.getSource() << std::endl << "\t";
			// 	for (map<string, vector<string> >::iterator maps = reg->second.begin(); maps != reg->second.end(); maps++)
			// 	{
			// 		std::cout << maps->first << std::endl << "\t\t";
			// 		for (vector<string>::iterator vec = maps->second.begin(); vec != maps->second.end(); vec++)
			// 			std::cout << *vec << " ";
			// 		std::cout << std::endl;
			// 	}
			// }

			// for (map<string, map<string, vector<string> > >::iterator reg = conf.begin(); reg != conf.end(); reg++)
			// {
			// 	std::cout << "DEBUG LOCATION: " << reg->first << std::endl << "\t";
			// 	for (map<string, vector<string> >::iterator maps = reg->second.begin(); maps != reg->second.end(); maps++)
			// 	{
			// 		std::cout << maps->first << std::endl << "\t\t";
			// 		for (vector<string>::iterator vec = maps->second.begin(); vec != maps->second.end(); vec++)
			// 			std::cout << *vec << " ";
			// 		std::cout << std::endl;
			// 	}
			// }

			/* Init stuct Host */
			Host temp_host = {
				this->checkPort(port, p_filname[i]),
				this->checkRoot(tmp, p_filname[i]),
				this->checkAutoIndex(tmp, p_filname[i]),
				this->convertIndex(tmp),
				this->checkServerName(tmp, p_filname[i]),
				this->checkErrorPage(mapError, p_filname[i], tmp.at("root")),
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
	size_t				nb(0);
	int					nbLine(0);
	std::map<string, string>::iterator		it = http.begin();

	while (getline(file, line))
	{
		string				key;
		string				arg;
		std::stringstream	str(line);

		nbLine++;
		str >> key;
		if (str.eof())
			continue;
		if (key.at(0) == '#')
			continue;
		if ((nb = key.find_first_of(':') != string::npos))
			key.erase(key.find_first_of(':'), nb + 1);
		this->checkKeyIsNotValid(key, &nbLine, _fileConf);
		getline(str, arg);
		if ((nb = arg.find_first_of(';')) != string::npos)
		{
			if (!checkEndLine(arg.substr(nb, arg.size()), ";"))
				throw std::invalid_argument("Error: line " + toStr(nbLine) + " not finish.");
			arg.erase(nb, arg.size());
		}
		if ((nb = arg.find_first_of(';')) != string::npos)
			arg.erase(nb, arg.size());
		if ((nb = arg.find_first_not_of(' ')) != string::npos)
			arg.erase(0, nb);
		this->checkKeyExist(key, http);
		http.insert(it, std::pair<string, string>(key, arg));
	}
}

void					ServerConfig::checkIfParamsExist( void )
{
	if (http.find("worker_processes") != http.end())
	{
		uint16_t nb = tryParseInt(http.at("worker_processes"));
		if (nb < 1 || nb > 100)
			throw std::invalid_argument("Error: Invalid argument:'worker_processes' must be 1 and 100.");
	}
	else
		http["worker_processes"] = "8";
	if (http.find("uri_max_size") == http.end())
	{
		errno = EINVAL;
		throw configException("Error in config file with params uri_max_size not exist");
	}
	else
		tryParseInt(http.at("uri_max_size"));
	if (http.find("max_empty_line_before_request") == http.end())
	{
		errno = EINVAL;
		throw configException("Error in config file with params max_empty_line_before_request not exist");
	}
	else
		tryParseInt(http.at("max_empty_line_before_request"));
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
