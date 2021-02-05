#ifndef UTILS_HPP
# define UTILS_HPP
# include <string>
# include <sstream>

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

#endif