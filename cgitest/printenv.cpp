#include <iostream>
#include <string.h>

using namespace std;

int main(int ac, char ** argv, char **env)
{
	(void)ac; (void)argv;
	while (*env)
		cout << *(env++) << endl;
	return (0);
}
