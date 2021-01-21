#include <iostream>
#include <string.h>

using namespace std;

int main(int ac, char ** argv, char **env)
{
	return (0);
	(void)ac; (void)argv;
	while (*env)
		cout << *(env++) << endl;
	while (*argv)
		cout << *(argv++) << endl;
	return (0);
}
