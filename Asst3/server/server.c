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
#include<openssl/sha.h>

typedef struct node
{
    char* data;
    struct node* next;
}node;

typedef struct file
{
	int version;
	char* filename;
	char* digest;
	//char* path;
	struct file* next;
}file;

typedef struct manifest
{
	int version;
	file** files;

}manifest;

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

int recurse_del(char* file)
{
    struct dirent* dp;
    DIR* dir = opendir(file);
    dp = readdir(dir);
    dp = readdir(dir);
    dp = readdir(dir);
    if(dp == NULL)
        return 1;
    char* temp;
    while(dp!=NULL)
    {
        if(dp->d_type == 4)
        {
            temp = myMalloc(strlen(file)+256);
            sprintf(temp,"%s/%s",file,dp->d_name);
            if(recurse_del(temp))
                remove(temp);
            free(temp);
        }
        else if(dp->d_type == 8)
        {
            temp = myMalloc(strlen(file)+256);
            sprintf(temp,"%s/%s",file,dp->d_name);
            remove(temp);
            free(temp);
        }
        dp = readdir(dir);
    }
    return 1;
}

void serv_del(int client_sock)
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
        printf("Something went wrong with delete.\n");
        exit(1);
    }
    char* fileName = myMalloc(siz);
    i=0;
    for(i;i<siz;i++)
    {
        read(client_sock,c,1);
        strcat(fileName,c);
    }
    if(fileLookup(fileName))
    {
        if(recurse_del(fileName))
            remove(fileName);
        write(client_sock,"1",1); 
    }
    else
        write(client_sock,"0",1);
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
    else if(strcmp(flag,"DEL")==0)
        serv_del(client_sock);
    free(c);
}

char* getDigest(char* file)
{
	int fd = open(file, O_RDONLY);
	if(fd < 0)
		close(fd);
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	char* c = myMalloc(sizeof(char));
	while(read(fd, c, 1) > 0)
	{
		SHA1_Update(&ctx, c, 1);
	}
	unsigned char tmphash[SHA_DIGEST_LENGTH];
	SHA1_Final(tmphash, &ctx);

	char* hash = myMalloc(SHA_DIGEST_LENGTH*2);

	int i = 0;
	for(i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
  	  sprintf((char*)&(hash[i*2]), "%02x", tmphash[i]);
	}
	free(c);
	close(fd);
	return hash;
}

file* initFile(char* filename)
{
	file* temp = myMalloc(sizeof(file));
	temp->version = 1;
	temp->filename = filename;
	temp->digest = getDigest(filename);
	//temp->path = NULL;//getPath(filename); NEEDS TO BE MADE
	temp->next = NULL;

	return temp;
}

manifest* initManifest()
{
	manifest* temp = myMalloc(sizeof(manifest));
	temp->version = 0;
	temp->files = myMalloc(20*sizeof(file*));

	return temp;
}

void freeManifest(manifest* m)
{
	int i;
	for(i = 0; i < 20; i++)
	{
		file* temp = m->files[i];
		while(temp != NULL)
		{
			file* temp2 = temp;
			temp = temp->next;
			free(temp2->digest);
			free(temp2);
		}
	}	
	free(m);
}

int getASCII(char* filename)
{
	int value = 0;
	int i;
	for(i = 0; i < strlen(filename); i++)
	{
		value += (int)filename[i];
	}
	return value;
}

manifest* insertToManifest(manifest* m, char* filename)
{
	int bucket = (getASCII(filename)) % 20;
	file* temp = initFile(filename);
	temp->next = m->files[bucket]; 
	m->files[bucket] = temp;
	return m;
}

manifest* updateManifest(manifest* m, char* filename)
{
	int bucket = (getASCII(filename)) % 20;
	file* ptr;
	for(ptr = m->files[bucket]; ptr != NULL; ptr = ptr->next){
		if(strcmp(ptr->filename, filename) == 0)
		{
			ptr->version++;
			ptr->digest = getDigest(filename);
			break;
		}
	}
	return m;
}

manifest* deleteFromManifest(manifest* m, char* filename)
{
	int bucket = (getASCII(filename)) % 20;
	if(m->files[bucket] == NULL)
		return m;
	file* curr = m->files[bucket];
	file* prev;
	if(strcmp(m->files[bucket]->filename, filename) == 0) //delete head
	{ 
		m->files[bucket] = curr->next;
		free(curr->digest);
		free(curr);
		return m;
	}
	while(curr != NULL && (strcmp(curr->filename, filename) != 0))
	{
		prev = curr;
		curr = curr->next;
	}
	if(curr == NULL)
		return m;
	prev->next = curr->next;
	free(curr->digest);
	free(curr);
	return m;
}

void createManifestFile(manifest* m)
{
	remove(".Manifest");			
	int wfd = open(".Manifest", O_WRONLY | O_APPEND | O_CREAT, 00777);	
	if(wfd < 0)
	{
		printf("Error, could not create .Manifest.\n");
		close(wfd);
		return;
	}
	char* num = myMalloc(100*sizeof(char));
	sprintf(num, "%d", m->version);
	write(wfd, num, strlen(num));
	write(wfd, "\n", 1);
	int i;
	for(i = 0; i < 20; i++)
	{
		file* temp = m->files[i];
		while(temp != NULL)
		{
			file* temp2 = temp;
			temp = temp->next;
			sprintf(num, "%d", temp2->version);
			write(wfd, num, strlen(num));
			write(wfd, "\t", 1);
			write(wfd, temp2->filename, strlen(temp2->filename));
			write(wfd, "\t", 1);
			write(wfd, temp2->digest, strlen(temp2->digest));
			write(wfd, "\n", 1);
		}
	}	
	free(num);
	close(wfd);
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

	/*
	char* file1 = argv[1];
	char* file2 = argv[2];
	
	manifest* m = initManifest();
	m = insertToManifest(m, file1);
	m = insertToManifest(m, file2);
	m = deleteFromManifest(m, file2);
	createManifestFile(m);
	freeManifest(m);
	*/

    return 0;
}
