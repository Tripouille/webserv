#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <string>
# include <map>

typedef int SOCKET;

using std::string;

struct Client
{
	struct authentication
	{
		string scheme;
		string ident;
		string user;
		string password;
	};

	Client(SOCKET _s, char const * _addr);
	Client(void);
	~Client();
	Client(Client const & other);
	Client & operator=(Client const & other);

	SOCKET s;
	string addr;
	std::map<string, authentication > authentications; // string = realm name
};

#endif