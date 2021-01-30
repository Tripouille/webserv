#ifndef BUFFERQ_HPP
# define BUFFERQ_HPP
# include <queue>
# include <list>
# include <iostream>
# include <string.h>

using std::list; using std::queue; using std::streamsize;

struct s_buffer
{
	char *		b;
	streamsize	size;
	streamsize	occupiedSize;
	
	s_buffer(streamsize s);
	s_buffer(s_buffer const & other);
	~s_buffer(void);
	private:
		s_buffer & operator=(s_buffer const &);
};

typedef queue<s_buffer *, list<s_buffer *> > t_bufferQ;

void deleteQ(t_bufferQ & q);

#endif
