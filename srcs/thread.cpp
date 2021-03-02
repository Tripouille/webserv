#include "thread.hpp"

void launchThreads(TcpListener * listener, fd_set const & readfds, fd_set const & writefds, int workerNb)
{
	pthread_t threads[workerNb];
	void * threadInfos[workerNb];
	bool potatoIsLife = true;

	for (int i = 0; i < workerNb; ++i) {
		threadInfos[i] = new s_threadInfos(listener, i, readfds, writefds, workerNb, &potatoIsLife);
		if (!potatoIsLife || pthread_create(threads + i, NULL, handleThread, threadInfos[i]) != 0)
			throw TcpListener::tcpException("Error in launching threads");
	}
	for (int i = 0; i < workerNb; ++i) {
		if (pthread_join(threads[i], NULL) != 0 || !potatoIsLife)
		{
			for (int j = i; j < workerNb; ++j)
				delete static_cast<s_threadInfos*>(threadInfos[j]);
			exit(EXIT_FAILURE);
		}
		delete static_cast<s_threadInfos*>(threadInfos[i]);
	}
}

void * handleThread(void * threadInfos)
{
	s_threadInfos * infos = (s_threadInfos*)threadInfos;
	try
	{
		infos->listener->handleSocket(infos->id, infos->readfds, infos->writefds, infos->workerNb);
	}
	catch (TcpListener::tcpException const & e)
	{
		cerr << e.what() << endl;
		*(infos->potatoIsLife) = false;
	}
	return (NULL);
}