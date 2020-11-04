#include <sys/socket.h>
#include <netinet/in.h>
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
using std::string;
using std::vector;

vector<string> split(string const& s, char delimitor)
{
	vector<string>	res;
	size_t			start = -1;
	size_t			end = 0;

	while (s[++start])
	{
		end = start;
		while (s[end] && s[end] != delimitor)
			++end;
		res.push_back(s.substr(start, end - start));
		start = end;
	}
	return (res);
}

int main(void)
{
	int				server_fd, new_socket;
	long			valread;
	sockaddr_in		address;
	int				addrlen = sizeof(address);

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

	std::cout << "++++++++ Waiting for new connection ++++++++" << std::endl << std::endl;
	if ((new_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen)) < 0)
	{
		perror("Error : in accept");
		exit(EXIT_FAILURE);
	}






	char buffer[1024] = {0};
	valread = recv(new_socket, buffer, 1024, 0);
	//std::cout << buffer << std::endl;
	vector<string> header = split(buffer, '\n');
	for_each(header.begin(), header.end(), [](string const & s){std::cout << s << std::endl;});
	vector<string> header_first_line = split(header[0], ' ');
	for_each(header_first_line.begin(), header_first_line.end(), [](string const & s){std::cout << s << std::endl;});


	string message = "HTTP/1.1 200 OK\n";
	if (valread < 0)
		std::cout << "No bytes are there to read" << std::endl;
	else if (header_first_line[1] == "/elephant.jpg")
	{
		message += "Content-Type: image/jpeg\nContent-Length: ";
		std::ifstream file("." + header_first_line[1]);
		if (!file.is_open())
		{
			perror("can't open file");
			return (-1);
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		message += std::to_string(buffer.str().size()) + "\n\n" + buffer.str();
		std::cout << "message : " << buffer.str().size() << std::endl;
	}
	else
		message += "Content-Type: text/plain\nContent-Length: 12\n\nHello world!";
	std::cout << "(message sent)" << std::endl;
	write(new_socket, message.c_str(), message.size());




	
	close(new_socket);
}

//includes en plus (https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html)
//mot-clé struct inutile (c > c++)
// read > recv