#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#define PORT 8080

int main(void)
{
	int sock = 0; long valread;
	struct sockaddr_in serv_addr;
	const char* hello = "Hello from client";
	char buffer[1024] = {0};

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cerr << "Socket creation error" << std::endl;
		return (-1);
	}
	
	memset(&serv_addr, '0', sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		std::cerr << "Invalid address/ Address not supported" << std::endl;
		return (-1);
	}
	
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cerr << "Connection failed" << std::endl;
		return (-1);
	}

	std::cout << "message : " << hello << std::endl;
	send(sock , hello , strlen(hello) , 0 );
	std::cout << "(hello message sent)" << std::endl;
	valread = read( sock , buffer, 1024);
	std::cout << buffer << std::endl;
	return 0;
}