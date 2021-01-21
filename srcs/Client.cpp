#include <Client.hpp>

Client::Client(SOCKET _s, char const * _addr)
	   : s(_s), addr(_addr)
{
}

Client::Client(void)
{
}

Client::~Client(void)
{
}

Client::Client(Client const & other) : s(other.s), addr(other.addr),
									   authentications(other.authentications)
{
}

Client &
Client::operator=(Client const & other)
{
	if (this != &other)
	{
		s = other.s;
		addr = other.addr;
		authentications = other.authentications;
	}
	return (*this);
}