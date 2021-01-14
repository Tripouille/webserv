#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <string>

typedef int SOCKET;

using std::string;

struct Client
{
	Client(void);
	~Client();
	Client(Client const & other);
	Client & operator=(Client const & other);

	SOCKET s;
	string auth;
	string addr;
	string ident;
	string user;
};

#endif