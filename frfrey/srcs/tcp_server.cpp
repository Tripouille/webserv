#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#define PORT 8080
#define MAXMSG 512
using std::string;
using std::vector;

vector<string> split(std::string s, char delimitor)
{
	vector<string>	res;
	size_t			start = 0;
	size_t			end = 0;

	while (s[start])
	{
		end = start;
		while (s[end] && s[end] != delimitor)
			++end;
		res.push_back(s.substr(start, end - start));
		start = end;
		while (s[start] == delimitor)
			++start;
	}
	return (res);
}

void read_data_and_answer(char const * buffer, int fd)
{
	vector<string> header = split(string(buffer), '\n');
	std::cerr << std::endl << "Header received : " << std::endl;
	for_each(header.begin(), header.end(), [](string const & s){std::cout << s << std::endl;});
	std::cerr << std::endl;
	vector<string> header_first_line = split(header[0], ' ');
	//for_each(header_first_line.begin(), header_first_line.end(), [](string const & s){std::cout << s << std::endl;});


	string message = "HTTP/1.1 200 OK\n";
	if (header_first_line.size() < 2)
		return ;
	if (header_first_line[1] == "/elephant.jpg")
	{
		message += "Content-Type: image/jpeg\nContent-Length: ";
		std::ifstream file("." + header_first_line[1]);
		if (!file.is_open())
		{
			perror("can't open file");
			return ;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		message += std::to_string(buffer.str().size()) + "\n\n" + buffer.str();
	}
	else if (header_first_line[1] != "/")
	{
		message += "Content-Type: text/html\nContent-Length: ";
		std::ifstream file("." + header_first_line[1]);
		if (!file.is_open())
		{
			message = "HTTP/1.1 404\n";
			message += "Content-Type: text/html\nContent-Length: ";
			file.close();
			file.open("." + string("/404.html"));
			if (!file.is_open())
			{
				perror("can't open file");
				return ;
			}
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		message += std::to_string(buffer.str().size()) + "\n\n" + buffer.str();
	}
	else
	{
		message += "Content-Type: text/html\nContent-Length: ";
		std::ifstream file("." + string("/index.html"));
		if (!file.is_open())
			std::ifstream file("." + string("/404.html"));
		std::stringstream buffer;
		buffer << file.rdbuf();
		message += std::to_string(buffer.str().size()) + "\n\n" + buffer.str();
	}
	send(fd, message.c_str(), message.size(), 0);
}

int main(void)
{
	int				server_fd, new_socket;
	sockaddr_in		address;
	int				addrlen = sizeof(address);
	fd_set			active_fd_set, read_fd_set;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Error : cannot create socket");
		return (0);
	}

	memset((char*)&address, 0, addrlen);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0)
	{
		perror("Error : bind failed");
		return (0);
	}

	if (listen(server_fd, 3) < 0)
	{
		perror("Error : in listen");
		exit(EXIT_FAILURE);
	}

	/*Four  macros are provided to manipulate the sets.  FD_ZERO() clears a set.  FD_SET()
       and FD_CLR() respectively add and  remove  a  given  file  descriptor  from  a  set.
       FD_ISSET()  tests  to  see  if  a file descriptor is part of the set; this is useful
       after select() returns.*/
	FD_ZERO(&active_fd_set);
	FD_SET(server_fd, &active_fd_set); //places server_fd in the fd_set active_fd_set

	while (1)
	{
		// Block until input arrives on one or more active sockets
		read_fd_set = active_fd_set;
		// int select(int ndffs, fd_set *readfds, fd_set *writefds, fd_sert *exceptfds, struct timeval* timeout)
		// timeout = NULL > infinite
		timeval tv = {10, 0}; // a remplacer par NULL
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, &tv) < 0)
		{
			perror("in select");
			exit(EXIT_FAILURE);
		}

		// Service all the sockets with input pending
		for (int i = 0; i < FD_SETSIZE; ++i)
		{
			if (FD_ISSET(i, &read_fd_set))
			{
				if (i == server_fd)
				{
					// Connection request on original socket
					std::cerr << std::endl << "Connection request on original socket" << std::endl;
					if ((new_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen)) < 0)
					{
						perror("in accept");
						exit(EXIT_FAILURE);
					}
					std::cerr << "Server : connect from host " << inet_ntoa(address.sin_addr)
							<< ", port " << ntohs(address.sin_port) << std::endl;
					FD_SET(new_socket, &active_fd_set);
				}
				else
				{
					// Data arriving on an already-connected socket
					std::cerr << std::endl << "Data arriving on an already connected socket" << std::endl;
					char buffer[MAXMSG];
					int nbytes;

					if ((nbytes = recv(i, buffer, MAXMSG, 0)) < 0)
					{
						perror("in read");
						//exit(EXIT_FAILURE);
					}
					else if (nbytes == 0) // end of file
					{
						close(i);
						FD_CLR(i, &active_fd_set);
					}
					else
					{
						buffer[nbytes - 1] = 0;
						read_data_and_answer(buffer, i);
					}
				}
			}
		}
	}
}
