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

int length;

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

void check(int num, char* msg)
{
    if(num < 0)
    {
        printf("%s\n",msg);
        exit(1);
    }
}

void config(char* ip,char* port)
{
    //Override previous config file
    remove(".configure");
    //check valid ip length
    if(strlen(ip) > 255 || strlen(ip) == 0)
    {
        printf("Error: ip adress is not valid. Aborting\n");
        exit(1);
    }
    //check valid port
    if(atoi(port) == 0)
    {
        printf("Error: port must be an integer. Aborting\n");
        exit(1);
    }
    else
    {
        if(atoi(port) > 65535 || atoi(port) < 1024)
        {
            printf("Error: port must be between 1024 and 65535\n");
            exit(1);
        }
    }
    //create file
    int wfd = open(".configure", O_WRONLY | O_APPEND | O_CREAT,00600);
    check(wfd,"Error: Could not create file in this directory");
    //write input to file
    write(wfd,ip,strlen(ip));
    write(wfd,"\t",1);
    write(wfd,port,strlen(port));
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

node* readFile(char* file)
{
    int fd = open(file,O_RDONLY);
    check(fd,"Error: Could not open file.");
    char* c = myMalloc(sizeof(char));
    node* head = NULL;                  //linked list of chars to create a token
    node* temp;
    node* list = NULL;                  //linked list of tokens from file
    length = 0;
    while(read(fd,c,1) > 0)
    {
        if(*c != '\t' && *c != '\n')
        {
            length+=1;
            temp = initNode();
            temp->data = myMalloc(sizeof(char));
            memcpy(temp->data,c,1);
            head = insert(head,temp);
        }
        else
        {
            list = addToList(list,head);
            length=0;
            freeList(head);
            head = NULL;
        }
    }
    if(length > 0)
        list = addToList(list,head);
    freeList(head);
    if(list == NULL)
    {
        printf("Error: file is empty\n");
        close(fd);
        exit(1);
    }
    free(c);
    return list;
}

int initSocket()
{
    node* list = readFile(".configure");
    if(list == NULL)
    {
        printf("Error: .configure is either empty, or could not be read. Aborting.\n");
        exit(1);
    }
    node* temp = list->next;
    if(temp == NULL)
    {
        printf("Error: .configure file is not formatted properly. Aborting.\n");
        exit(1);
    }
    char* ip = myMalloc(256);
    memcpy(ip,list->data,strlen(list->data));
    char* port = myMalloc(6);
    memcpy(port,temp->data,strlen(temp->data));
    int port_num = atoi(port);
    //init socket
    int sock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in server;
    bzero((char*)&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port_num);
    if(strcmp(ip,"INADDR_ANY")==0)
        server.sin_addr.s_addr = INADDR_ANY;
    else
    {
        struct hostent* host = gethostbyname(ip);
        if(host == NULL)
        {
            printf("Error: No such host\n");
            return -1;
        }
        bcopy((char*)host->h_addr,(char*)&server.sin_addr.s_addr,host->h_length);
    }
    int status = connect(sock,(struct sockaddr*)&server,sizeof(server));
    check(status,"There was an error making a connection to the server.");
    return sock;
}

void client_creat(char* file,int sock)
{
    //max file name length is 255, need 4 for create flag, need 4 max for number of bytes
    char* buf = myMalloc(300);
    char* ans = myMalloc(1);
    //construct a message in the format CRT~#bytes~filename
    sprintf(buf,"CRT~%d~%s",strlen(file),file);
    write(sock,buf,strlen(buf));
    printf("Finished writing to socket\n");
    read(sock,ans,1);
    if(atoi(ans) == 0)
    {
        printf("Project %s was created.\n",file);
        mkdir(file,00777);
        sprintf(buf,"%s/.Manifest",file);
        int fd = open(buf, O_WRONLY|O_APPEND|O_CREAT,00777);
        check(fd,"Error: Could not create a .Manifest for this project.");
        write(fd,"1\n",2);
    }
    else
        printf("The project alrady exists.\n");
    return;
}

void client_del(char* file,int sock)
{
    //max message length
    char* buf = myMalloc(300);
    char* ans = myMalloc(1);
    sprintf(buf,"DEL~%d~%s",strlen(file),file);
    write(sock,buf,strlen(buf));
    printf("Finished writing to socket\n");
    //read response
    read(sock,ans,1);
    if(atoi(ans) == 0)
        printf("Error: Project %s does not exist.\n",file);
    else
        printf("Project %s has been deleted.\n",file);
    return;
}

void add(char* project,char* file)
{
    struct dirent* dp;
    DIR* dir = opendir(project);
    dp = readdir(dir);
    dp = readdir(dir);
    dp = readdir(dir);
    //check if project exists
    if(dp==NULL)
    {
        printf("Error: The project %s does not exist.\n",project);
        return;
    }
    char* path = myMalloc(strlen(project)+strlen(file)+1);
    sprintf(path,"%s/%s",project,file);
    int fd = open(path,O_RDONLY);
    //check if file exists
    check(fd,"Error: The file or path provided does not exist. Aborting.");
    sprintf(path,"%s/.Manifest",project);
    while(dp!=NULL)
    {
        if(strcmp(dp->d_name,".Manifest")==0)
        {
            fd = open(path, O_WRONLY | O_APPEND);
            check(fd,"Error: Could not find .Manifest, Aborting.");
            char* buf = myMalloc(512);
            sprintf(buf,"~\t1\t%s/%s",project,file);
            write(fd,buf,strlen(buf));
        }
        dp = readdir(dir);
    }
}

void rem(char* project,char* file)
{
    struct dirent* dp;
    DIR* dir = opendir(project);
    dp = readdir(dir);
    dp = readdir(dir);
    dp = readdir(dir);
    //check if project exists
    if(dp==NULL)
    {
        printf("Error: The project %s does not exist.\n",project);
        return;
    }
    char* path = myMalloc(strlen(project)+strlen(file)+1);
    sprintf(path,"%s/%s",project,file);
    char* man = myMalloc(strlen(project) + 10);
    sprintf(man,"%s/.Manifest",project);
    while(dp!=NULL)
    {
        if(strcmp(dp->d_name,".Manifest")==0)
        {
            node* list = readFile(man);
            node* ptr = list;
            node* prev = NULL;
            node* prev2 = NULL;
            node* prev3 = NULL;
            while(ptr!=NULL)
            {
                if(strcmp(ptr->data,path)==0)
                {
                    if(strcmp(prev2->data,"~")== 0)
                    {
                        prev3->next = ptr->next;
                        ptr->next = NULL;
                        free(prev2);
                    }
                    else
                    {
                        prev2->next = ptr->next;
                        ptr->next = NULL;
                        free(prev);
                    }
                    remove(man);
                    int fd = open(man, O_WRONLY | O_APPEND | O_CREAT,00777);
                    check(fd,"Error: Could not update .Manifest file. Aborting.");
                    int i = 1;
                    int extra = 0;
                    ptr = list->next;
                    write(fd,list->data,strlen(list->data));
                    write(fd,"\n",1);
                    while(ptr!=NULL)
                    {
                        write(fd,ptr->data,strlen(ptr->data));
                        if(strcmp(ptr->data,"~")==0 || atoi(ptr->data)>0)
                            write(fd,"\t",1);
                        else
                            write(fd,"\n",1);
                        ptr=ptr->next;
                    }
                    printf("Removed %s from %s\n",file,project);
                    return;
                }
                prev3 = prev2;
                prev2 = prev;
                prev = ptr;
                ptr = ptr->next;
            }
        }
        dp = readdir(dir);
    }
    printf("Error: %s does not exist in %s\n",file,project);
    return;
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
	remove("Manifest");			
	int wfd = open("Manifest", O_WRONLY | O_APPEND | O_CREAT,00600);	//WHY can't i create a .Manifest file
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
    if(argc == 4)
    {
        if(strcmp(argv[1],"configure")==0)
            config(argv[2],argv[ 3]);
        else if(strcmp(argv[1],"add")==0)
            add(argv[2],argv[3]);
        else if(strcmp(argv[1],"remove")==0)
            rem(argv[2],argv[3]);
        return 0;
    }
    else if(argc == 3)
    {
        if(strcmp(argv[1],"configure")==0)
            printf("Error, expected 4 arguments, received %d\n",argc);
        else if(strcmp(argv[1],"add")==0)
            printf("Error, expected 4 arguments, received %d\n",argc);
        if(strcmp(argv[1],"create")==0)
        {
            int net_sock = initSocket();
            if(net_sock<0)
                printf("Error, could not connect to server\n");
            client_creat(argv[2],net_sock);
        }
        else if(strcmp(argv[1],"delete")==0)
        {
            int net_sock = initSocket();
            if(net_sock<0)
                printf("Error, could not connect to server\n");
            client_del(argv[2],net_sock);
        }
        return 0;
    }
    int net_sock = initSocket();
    if(net_sock < 0)
    {
        printf("Could not connect to server");
        return 0;
    }
    char* response = myMalloc(256);
    recv(net_sock,response, sizeof(response),0);
    printf("The server sent back:\n%s",response);
    close(net_sock);

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
