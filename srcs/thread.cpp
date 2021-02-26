#include "thread.hpp"

void launchThreads(TcpListener * listener, fd_set const & readfds, fd_set const & writefds, int workerNb)
{
	pthread_t threads[workerNb];
	void * threadInfos[workerNb];
	for (int i = 0; i < workerNb; ++i) {
		threadInfos[i] = new s_threadInfos(listener, i, readfds, writefds, workerNb);
		if (pthread_create(threads + i, NULL, handleThread, threadInfos[i]) != 0)
			throw TcpListener::tcpException("Unable to create thread");
	}
	for (int i = 0; i < workerNb; ++i) {
		if (pthread_join(threads[i], NULL) != 0)
		{
			for (int j = i; j < workerNb; ++j)
				delete static_cast<s_threadInfos*>(threadInfos[j]);
			throw TcpListener::tcpException("Unable to join thread");
		}
		delete static_cast<s_threadInfos*>(threadInfos[i]);
	}
}

void * handleThread(void * threadInfos)
{
	s_threadInfos * infos = (s_threadInfos*)threadInfos;
	infos->listener->handleSocket(infos->id, infos->readfds, infos->writefds, infos->workerNb);
	return (NULL);
}