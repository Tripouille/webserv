#ifndef CGIREQUEST_HPP
# define CGIREQUEST_HPP

#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <vector>

using std::cout; using std::endl;

class CgiRequest
{
	public:
		CgiRequest(void);
		~CgiRequest(void);
		CgiRequest(CgiRequest const & other);

		CgiRequest & operator=(CgiRequest const & other);
		void doRequest(void);


	private:
		//CgiRequest(void);
		void _copy(CgiRequest const & other);

		enum {BUFFER_SIZE = 100000};

		char *	_env[6];
		char *	_av[1];
		char	_buffer[BUFFER_SIZE + 1];
};

#endif