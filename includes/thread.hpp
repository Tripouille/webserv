#ifndef THREAD_HPP
# define THREAD_HPP

# include "TcpListener.hpp"

class TcpListener;
struct s_threadInfos {
	s_threadInfos(TcpListener * l, size_t i, fd_set const & r, fd_set const & w, int nb, bool * p)
	: listener(l), id(i), readfds(r), writefds(w), workerNb(nb), potatoIsLife(p) {}
	TcpListener *	listener;
	size_t			id;
	fd_set const &	readfds;
	fd_set const &	writefds;
	int				workerNb;
	bool *			potatoIsLife;
};

void launchThreads(TcpListener * listener, fd_set const & readfds, fd_set const & writefds, int workerNb);
void * handleThread(void * threadInfos);

#endif