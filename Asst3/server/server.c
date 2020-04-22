#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <math.h>
#include <dirent.h>

typedef struct node
{
    char* data;
    struct node* next;
}node;

int length;

void* myMalloc(int size)
{
    void* temp = calloc(size,1);
    if(temp == NULL)
    {
        printf("Error: Malloc has returned null, Aborting.\n");
        exit(1);
    }
    return temp;
}

node* initNode()
{
    node* temp = myMalloc(sizeof(node));
    temp->next = NULL;
    temp->data = NULL;
    return temp;
}

void freeList(node* head)
{
    node* temp;
    while(head!=NULL)
    {
        temp = head;
        head = head->next;
        if(temp->data != NULL)
            free(temp->data);
        free(temp);
    }
}

node* insert(node* head, node* temp)
{
    node* curr = head;
    node* prev = NULL;
    while(curr != NULL)
    {
        prev = curr;
        curr = curr->next;
    }
    if(prev==NULL)
        head = temp;
    else
        prev->next = temp;
    return head;
}

node* addToList(node* head,node* word)
{
    char* w = myMalloc(length+1);
    int i;
    for(i = 0;i<length;i++)
    {
        w[i] = *word->data;
        word= word->next;
    }
    node* temp = initNode();
    if(length == 0)
        temp->data = "";
    else
        temp->data = w;
    head = insert(head,temp);
    return head;
}

int fileLookup(char* file)
{
    struct dirent* dp;
    DIR* dir = opendir(".");
    dp = readdir(dir);
    dp = readdir(dir);
    dp = readdir(dir);
    while(dp!=NULL)
    {
        if(dp->d_type == 4)
            if(strcmp(dp->d_name,file)==0)
                return 1;
        dp = readdir(dir);
    }
    return 0;
}

void serv_creat(int client_sock)
{
    //get size
    char* c = myMalloc(1);
    char* size = myMalloc(32);
    int i = 0;
    while(read(client_sock,c,1)>0 && *c != '~')
        strcat(size,c);
    //convert size to int
    int siz = atoi(size);
    if(siz <= 0)
    {
        printf("Something went wrong with create.\n");
        exit(1);
    }
    char* fileName = myMalloc(siz);
    i=0;
    for(i;i<siz;i++)
    {
        read(client_sock,c,1);
        strcat(fileName,c);
    }
    // read(client_sock,fileName,siz);
    //read siz amount of bytes for filename
    if(fileLookup(fileName))
        write(client_sock,"1",1);
    else
    {
        write(client_sock,"0",1);
        char* path = myMalloc(512);
        sprintf(path,"./%s",fileName);
        mkdir(path,00777);
        strcat(path,"/.Manifest");
        int fd = open(path, O_WRONLY | O_CREAT,00600);
        if(fd<0)
        {
            printf("Could not create project %s\n",fileName);
            return;
        }
    }
}

void handle_connection(int client_sock)
{
    char* flag = myMalloc(3);
    int i = 0;
    char* c = myMalloc(1);
    //get the flag
    while(read(client_sock,c,1) > 0 && *c != '~')
        strcat(flag,c);
    if(strcmp(flag,"CRT")==0)
        serv_creat(client_sock);
    free(c);
}

int main(int argc, char** argv)
{
    if(argc!=2)
    {
        printf("Error: expected 2 arguments, received %d\n",argc);
        return 0;
    }
    int port = atoi(argv[1]);
    if(port <= 0)
    {
        printf("Error: %s is not a valid port\n",argv[1]);
        return 0;
    }
    int serv_sock;
    serv_sock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bind(serv_sock,(struct sockaddr*)&server_addr,sizeof(server_addr));
    listen(serv_sock,1);
    int client_sock;
    client_sock = accept(serv_sock,NULL,NULL);
    handle_connection(client_sock);
    close(serv_sock);
    return 0;
}