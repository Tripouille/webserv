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

std::streamsize selectAndRead(SOCKET socket, char * buffer, size_t size)
{
	fd_set	fdSet;
	FD_ZERO(&fdSet);
	FD_SET(socket, &fdSet);
	int selectRet;
	std::streamsize bytesRead = 0;

	while (static_cast<size_t>(bytesRead) < size)
	{
		timeval timeout = {READ_TIMEOUT, 0};
		if ((selectRet = select(FD_SETSIZE, &fdSet, NULL, NULL, &timeout)) < 0)
		{
			cerr << "select error in selectAndRead" << endl;
			return (-1);
		}
		if (FD_ISSET(socket, &fdSet))
		{
			ssize_t readReturn = read(socket, buffer + bytesRead, size - static_cast<size_t>(bytesRead));
			if (readReturn < 0)
				return (readReturn);
			bytesRead += readReturn;
		}
		else
			return (bytesRead);
	}
	return (bytesRead);
}

std::streamsize selectAndWrite(SOCKET socket, char * buffer, size_t size)
{
	fd_set	fdSet;
	FD_ZERO(&fdSet);
	FD_SET(socket, &fdSet);
	int selectRet;
	std::streamsize bytesWritten = 0;

	fcntl(socket, F_SETFL, O_NONBLOCK);
	while (static_cast<size_t>(bytesWritten) < size)
	{
		timeval timeout = {WRITE_TIMEOUT, 0};
		if ((selectRet = select(FD_SETSIZE, NULL, &fdSet, NULL, &timeout)) < 0)
		{
			cerr << "select error in selectAndWrite" << endl;
			return (-1);
		}
		if (FD_ISSET(socket, &fdSet))
		{
			ssize_t writeReturn = write(socket, buffer + bytesWritten, size - static_cast<size_t>(bytesWritten));
			if (writeReturn > 0)
				bytesWritten += writeReturn;
		}
		else
			return (-1);
	}
	return (bytesWritten);
}

short			tryParseInt(std::string & str)
{
	if (str.find_first_not_of("0123456789", 0) != std::string::npos)
		throw std::invalid_argument("Error: Invalid argument: Params is not a valid number.");
	if (str.size() > 9)
		throw std::invalid_argument("Error: Invalid argument: Params is not a valid number.");
	return (atoi(str.c_str()));
}

bool				checkEndLine( std::string str, std::string comp)
{
	std::string			tmp;
	std::stringstream	line(str);
	size_t				pos(0);

	line >> tmp;
	if (tmp == comp && line.eof())
		return true;
	else
	{
		if (tmp[0] == '#')
			return true;
		if (tmp.find_first_not_of(";") != std::string::npos && !tmp.empty())
			return false;
		tmp.erase();
		line >> tmp;
		if ((pos = tmp.find_first_of('#')) != std::string::npos)
			tmp.erase(pos);
	}
	if (tmp.empty() && line.eof())
		return true;
	return false;
}


void sigPipeCatcher(int)
{
	cerr << "SIGPIPE catched and ignored" << endl;
}
