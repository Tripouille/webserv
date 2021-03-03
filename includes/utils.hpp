#ifndef UTILS_HPP
# define UTILS_HPP
# include <string>
# include <sstream>
# include <sys/socket.h>
# include <unistd.h>
# include <iostream>
# include <fcntl.h>
# include <stdexcept>

# define WRITE_TIMEOUT 1
# define READ_TIMEOUT 1

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

std::string		intToHex(int const & value);
ssize_t			hexToDec(std::string const & str);
bool			isHex(std::string const & str);

std::streamsize loopRecv(SOCKET socket, char * buffer, ssize_t size);
std::streamsize selectAndRead(SOCKET socket, char * buffer, size_t size);
std::streamsize selectAndWrite(SOCKET socket, char * buffer, size_t size);
short			tryParseInt(std::string & str);
bool			checkEndLine( std::string str, std::string comp);

void			sigPipeCatcher(int);

#endif
