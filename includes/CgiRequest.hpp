#ifndef CGIREQUEST_HPP
# define CGIREQUEST_HPP

#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <vector>
#include "BufferQ.hpp"
#define STDOUT 1

using std::cout; using std::endl; using std::string;

class CgiRequest
{

	class cgiException : public std::exception
	{
		public:
			cgiException(string str = "") throw();
			virtual ~cgiException(void) throw();
			virtual const char * what(void) const throw();
		private:
			string _str;
	};
	public:
		CgiRequest(void);
		~CgiRequest(void);
		CgiRequest(CgiRequest const & other);

		CgiRequest & operator=(CgiRequest const & other);
		void doRequest(void);
		t_bufferQ const & getAnswer(void) const;

	private:
		//CgiRequest(void);
		void _copy(CgiRequest const & other);

		enum {BUFFER_SIZE = 100000, TIMEOUT = 100000};

		char *		_env[6];
		char *				_av[1];
		t_bufferQ			_answer;	
};

#endif