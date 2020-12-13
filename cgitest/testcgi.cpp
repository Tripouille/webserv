
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

using namespace std;

int main(int ac, char ** argv)
{
	int status; (void)ac; (void)argv;
	int stdOutSave = dup(1);
	int p[2]; pipe(p);
	int child = fork();
	if (child == 0)
	{
		char ** env = new char *[6];
		env[0] = strdup("GATEWAY_INTERFACE=CGI/1.1");
		env[1] = strdup("SERVER_PROTOCOL=HTTP/1.1");
		env[2] = strdup("SCRIPT_FILENAME=./test.php");
		env[3] = strdup("SCRIPT_NAME=test.php");
		env[4] = strdup("REDIRECT_STATUS=200");
		env[5] = 0;
		char ** av = new char *[2]; av[0] = 0;
		dup2(p[1], 1);
		//if (execve("/usr/bin/php-cgi", av, env) == -1)
		if (execve("./printenv", av, env) == -1)
			exit(EXIT_FAILURE);
	}
	else
	{
		usleep(100000);
		waitpid(child, &status, 1);
		kill(child, SIGKILL);
		char buffer[100001];
		int readReturn = read(p[0], buffer, 100000);
		buffer[readReturn] = 0;
		dup2(stdOutSave, 1);
		//cout << "readReturn = " <<  readReturn << endl;
		cout << buffer << endl;
	}
	
	return (0);
}