#include <iostream>
#include <string.h>

using namespace std;

int main(int ac, char ** argv, char **env)
{
	while (1);
	return (0);
	(void)ac; (void)argv;
	while (*env)
		cout << *(env++) << endl;
	while (*argv)
		cout << *(argv++) << endl;
	return (0);
}
