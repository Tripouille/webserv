#include <Client.hpp>

Client::Client(void)
{
}

Client::~Client(void)
{
}

Client::Client(Client const & other) : s(other.s), auth(other.auth), addr(other.addr),
									   ident(other.ident), user(other.user)
{
}

Client &
Client::operator=(Client const & other)
{
	if (this != &other)
	{
		s = other.s;
		auth = other.auth;
		addr = other.addr;
		ident = other.ident;
		user = other.user;
	}
	return (*this);
}