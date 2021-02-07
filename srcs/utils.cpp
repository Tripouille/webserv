#include "utils.hpp"

/*template<class T>
std::string toStr(T const & value)
{
	std::ostringstream ss;
	ss << value;
	return (ss.str());
}*/

std::string intToHex(int const & value)
{
	std::ostringstream ss;
	ss << std::hex <<value;
	return (ss.str());
}

ssize_t hexToDec(std::string const & str)
{
	ssize_t result;
	std::stringstream ss;

	ss << str;
	ss >> std::hex >> result;
	return (result);
}

bool isHex(std::string const & str)
{
	return (str.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos);
}