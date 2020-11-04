#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#define PORT 8080

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
	std::cout << buffer << std::endl;
	if (valread < 0)
		std::cout << "No bytes are there to read" << std::endl;
	const char* hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
	std::cout << "(hello message sent)" << std::endl;
	write(new_socket, hello, strlen(hello));
	
	close(new_socket);
}

//includes en plus (https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html)
//mot-clé struct inutile (c > c++)
// read > recv