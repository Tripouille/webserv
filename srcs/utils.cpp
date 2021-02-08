#include "utils.hpp"

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

std::streamsize loopRecv(SOCKET socket, char * buffer, ssize_t size)
{
	int bytesRead = 0;
	while (bytesRead < size)
	{
		std::streamsize recvReturn = recv(socket, buffer + bytesRead, static_cast<size_t>(size - bytesRead), 0);
		if (recvReturn < 0)
			return (recvReturn);
		else if (recvReturn == 0)
			return (bytesRead);
		bytesRead += recvReturn;
	}
	return (bytesRead);
}