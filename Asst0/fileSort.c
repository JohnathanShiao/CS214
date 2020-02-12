#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

typedef struct nodes{
    char* val;
    struct node* next;
}node;

int main(int argc,char** argv)
{
    char* file = argv[2];
    int fd = open(file,O_RDONLY);
    char* line = malloc(101);
    read(fd,line,100);
    printf("%s\n",line);
    return 0;
}