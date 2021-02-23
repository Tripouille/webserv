#ifndef UTILS_HPP
# define UTILS_HPP
# include <string>
# include <sstream>
# include <sys/socket.h>
# include <unistd.h>
# include <iostream>
# include <fcntl.h>
# include <stdexcept>

typedef int SOCKET;
using std::cerr;
using std::endl;

template<class T>
std::string toStr(T const & value)
{
	std::ostringstream ss;
	ss << value;
	return (ss.str());
}

std::string intToHex(int const & value);
ssize_t hexToDec(std::string const & str);
bool isHex(std::string const & str);

std::streamsize loopRecv(SOCKET socket, char * buffer, ssize_t size);
std::streamsize selectAndRead(SOCKET socket, char * buffer, size_t size);
std::streamsize selectAndWrite(SOCKET socket, char * buffer, size_t size);
uint16_t		tryParseInt(std::string & str);
bool			checkEndLine( std::string str, std::string comp);

#endif
