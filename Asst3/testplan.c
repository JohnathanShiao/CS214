#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <iostream>

int main(int argc, char** argv)
{
	char command[50];
	strcpy(command, "ls -a");
	system(command);

	return 0;
}
