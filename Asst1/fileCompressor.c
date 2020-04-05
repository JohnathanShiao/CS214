#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int length;
int recursive;

typedef struct node
{
    int count;
    char* data;
    struct node* left;
    struct node* right;
}node;

typedef struct LLNode{
	char* data;
	int freq;
	struct LLNode* next;
}LLNode;s

typedef struct list
{
    char* data;
    struct list* next;
}list;

void* myMalloc(int size)
{
    void* temp = calloc(1,size);
    if(temp == NULL)
    {
        printf("Error, malloc has returned NULL, Aborting now.\n");
        exit(1);
    }
    return temp;
}

node* initNode()
{
    node* temp = myMalloc(sizeof(node));
    temp->left = NULL;
    temp->right = NULL;
    temp->count = 0;
    return temp;
}

void freeNode(node* root)
{
    if(root == NULL)
        return;
    freeNode(root->left);
    freeNode(root->right);
    if(root->data != NULL)
        free(root->data);
    free(root);
}

list* initList()
{
    list* temp = myMalloc(sizeof(list));
    temp->next = NULL;
    temp->data = NULL;
    return temp;
}

void freeList(list* head)
{
    list* temp;
    while(head!=NULL)
    {
        temp = head;
        head = head->next;
        if(temp->data != NULL)
            free(temp->data);
        free(temp);
    }
}

list* insert(list* head, list* temp)
{
    list* curr = head;
    list* prev = NULL;
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

list* addToToken(list* head,list* word)
{
    char* w = myMalloc(length * sizeof(char));
    int i;
    for(i = 0;i<length;i++)
    {
        w[i] = *word->data;
        word = word->next;
    }
    list* temp = initList();
    temp->data = w;
    head = insert(head,temp);
    return head;
}

void decompress(node* root, char* file)
{
    int fd = open(file,O_RDONLY);
    if (fd < 0)
    {
        printf("Error, could not find %s in this directory.\n",file);
        close(fd);
        return;
    }
    int len = strlen(file);
    char* fileName = myMalloc(len-4);
    memcpy(fileName,file,len-4);
    int wfd = open(fileName, O_WRONLY | O_APPEND | O_CREAT,00600);
    if (wfd < 0)
    {
        printf("Error, could not create a new file named %s in this directory.\n",fileName);
        close(fd);
        close(wfd);
        return;
    }
    char* c = myMalloc(sizeof(char));
    node* ptr = root;
    while(read(fd,c,1) > 0)
    {
        if(*c == '0')
        {
            ptr = ptr->left;
            if(ptr->data != NULL)
            {
                write(wfd,ptr->data,strlen(ptr->data));
                ptr = root;
            }
        }
        else if (*c == '1')
        {
            ptr = ptr->right;
            if(ptr->data != NULL)
            {
                write(wfd,ptr->data,strlen(ptr->data));
                ptr = root;
            }
        }
        else
            ptr = root;
    }
    free(c);
    free(fileName);
    close(fd);
    close(wfd);
}

int isControl(char* temp,char* escape)
{
    int i = 0;
    for(i;i<strlen(escape);i++)
    {
        if(temp[i] != escape[i])
            return 0;
    }
    return 1;
}

node* genTree(list* head,char* escape)
{
    node* root = initNode();
    node* temp = root;
    list* ptr = head;
    int count = 0;
    while(ptr!=NULL)
    {
        ptr=ptr->next;
        count++;
    }
    if(count%2!=0)
    {
        printf("Error: Key value pairs are improperly formatted, aborting.\n");
        return NULL;
    }
    ptr = head->next;
    list* prev = head;
    int i = 0;
    while(ptr != NULL && prev != NULL)
    {
        char* path = prev->data;
        while(path[i])
        {
            if(path[i] == '0')
            {
                if(temp->left == NULL)
                    temp->left = initNode();
                temp = temp->left;
            }
            else
            {
                if(temp->right == NULL)
                    temp->right = initNode();
                temp=temp->right;
            }
            i++;
        }
        if(isControl(ptr->data,escape))
        {
            temp->data = myMalloc(1);
            if(ptr->data[strlen(ptr->data)-1] == 't')
                temp->data[0] = '\t';
            else
                temp->data[0] = '\n';
        }
        else
        {
            temp->data = myMalloc(strlen(ptr->data)+1);
            memcpy(temp->data,ptr->data,strlen(ptr->data)+1);
        }
        i=0;
        if(ptr->next == NULL)
            break;
        prev = ptr->next;
        ptr = prev->next;
        temp = root;
    }
    freeList(head);
    free(escape);
    return root;
}

node* loadBook(char* file)
{
    int fd = open(file, O_RDONLY);
    if(fd<0)
    {
        printf("Could not find the codebook %s in this directory.\n",file);
        close(fd);
        return NULL;
    }
    char* c = myMalloc(sizeof(char));
    list* head = NULL;                  //linked list of chars to create a token
    list* temp;
    list* token = NULL;                  //linked list of tokens from file
    length = 0;
    while(read(fd,c,1) > 0)
    {
        if(*c != '\n' && *c != '\t')
        {
            length+=1;
            temp = initList();
            temp->data = myMalloc(sizeof(char));
            memcpy(temp->data,c,1);
            head = insert(head,temp);
        }
        else
        {
            if(length!=0)
            {
                token = addToToken(token,head);
                length=0;
                freeList(head);
                head = NULL;
            }
        }
    }
    if(token == NULL)
    {
        printf("Error: codebook is empty, unable to decompress.\n");
        close(fd);
        return NULL;
    }
    free(c);
    close(fd);
    char* escape = myMalloc(32);
    memcpy(escape,token->data,strlen(token->data)+1);
    temp = token;
    token = token->next;
    free(temp->data);
    free(temp);
    return genTree(token,escape);
}

int encode(char* word, node* root, char* code,int i)
{
    if(root == NULL)
        return 0;
    if(root->data != NULL)
        if(strcmp(word,root->data) == 0)
            return 1;
    if(encode(word, root->left,code,i+1))
    {
        code[i] = '0';
        return 1;
    }
    else if(encode(word,root->right,code,i+1))
    {
        code[i] = '1';
        return 1;
    }
}

void compress(char* path, node* root)
{
    int fd = open(path,O_RDONLY);
    if (fd < 0)
    {
        printf("Error, could not find %s in this directory.\n",path);
        close(fd);
        return;
    }
    int len = strlen(path);
    char* fileName = myMalloc(len+4);
    memcpy(fileName,path,len);
    memcpy(fileName+len,".hcz",4);
    int wfd = open(fileName, O_WRONLY | O_APPEND | O_CREAT,00600);
    if (wfd < 0)
    {
        printf("Error, could not create a new file named %s in this directory.\n",fileName);
        close(fd);
        close(wfd);
        return;
    }
    char* c = myMalloc(sizeof(char));
    char* code = myMalloc(8);
    node* ptr = root;
    list* token = NULL;
    list* temp;
    list* head = NULL;
    while(read(fd,c,1) > 0)
    {
       if(*c != '\n' && *c != '\t' && *c != ' ')
        {
            length+=1;
            temp = initList();
            temp->data = myMalloc(sizeof(char));
            memcpy(temp->data,c,1);
            head = insert(head,temp);
        }
        else
        {
            if(length!=0)
            {
                token = addToToken(token,head);
                encode(token->data,root,code,0);
                if(strlen(code) == 0)
                {
                    printf("Error, something does not exist in the codebook, Aborting.\n");
                    exit(1);
                }
                write(wfd,code,strlen(code));
                free(code);
                code = myMalloc(8);
                length=0;
                freeList(head);
                freeList(token);
                token = NULL;
                head = NULL;
            }
            if(*c == ' ' || *c == '\n' || *c == '\t')   
            {
                encode(c,root,code,0);
                if(strlen(code) == 0)
                {
                    printf("Error, a delimiter does not exist in the codebook, Aborting.\n");
                    exit(1);
                }
                write(wfd,code,strlen(code));
                free(code);
                code = myMalloc(8);
            }
        }
    }
    if(length > 0)
        token = addToToken(token,head);
    freeList(head);
    encode(token->data,root,code,0);
    if(strlen(code) == 0)
    {
        printf("Error, something does not exist in the codebook, Aborting.\n");
        exit(1);
    }
    write(wfd,code,strlen(code));
    free(code);
    freeList(token);
    free(c);
    free(fileName);
    close(fd);
    close(wfd);
}

LLNode** insert_hash(LLNode** hash_table, char* string, int ascii_value)
{
	//printf("String: %s\tASCIIVal: %d\n", string, ascii_value);
	int bucket = ascii_value % 5; //5 = number of elements for testing
	LLNode* ptr;
	for(ptr = hash_table[bucket]; ptr != NULL; ptr = ptr->next){
		if(strcmp(ptr->data, string) == 0){
			ptr->freq += 1;
			return hash_table;
		}
	}
	LLNode* temp = malloc(sizeof(LLNode));
	temp->data = string;
	temp->freq = 1;
	temp->next = hash_table[bucket]; 
	hash_table[bucket] = temp;
	return hash_table;
}

LLNode** build_hashtable(int fd){
	LLNode** hash_table = myMalloc(5*sizeof(LLNode*));  //5 buckets for testing
	char* c = myMalloc(sizeof(char)); 
	list* token = NULL;
	list* temp;
	int length = 0;
	int ascii_value = 0;
	while(read(fd, c, 1) > 0)
	{
		if(*c != ' ' && *c != '\n' && *c != '\t')
		{
			length += 1;
			temp = initList();
			temp->data = myMalloc(sizeof(char));
			memcpy(temp->data, c, 1);
			token = insert(token, temp);
		}else
		{
			if(length != 0)
			{
				char* str = myMalloc(length * sizeof(char));
				int i;
				for(i = 0; i < length; i++)
				{
					str[i] = *token->data;
					ascii_value += (int)str[i];
					token = token -> next;
				}
				hash_table = insert_hash(hash_table, str, ascii_value);
				length = 0;
				ascii_value = 0;
				freeList(token);
				freeList(temp);

			}
		}
	}
	free(c);
	return hash_table;
}

void free_hash(LLNode** hash_table)
{
	int i;
	for(i = 0; i < 5; i++) //5buckets
	{
		LLNode* temp = hash_table[i];
		while(temp != NULL)
		{
			LLNode* temp2 = temp;
			temp = temp->next;
			printf("\n%s\t%d\n", temp2->data, temp2->freq);
			free(temp2->data);
			free(temp2);
		}	
	}
}

int main(int argc, char** argv)
{
    if(argc < 2 || argc > 5)
    {
        printf("Error: Expected 2-4 arguments, received %d\n",argc);
        return 0;
    }
    if(argc == 5)
        recursive = 1;
    else
        recursive = 0;
    char* flag = argv[1+(argc-4)];
    char* path = argv[2 + (argc-4)];
    if(strcmp(flag,"-d") == 0)
    {
        char* book = argv[3 + (argc-4)];
        node* root = loadBook(book);
        decompress(root,path);
        freeNode(root);
    }
    else if(strcmp(flag,"-c") == 0)
    {
        char* book = argv[3 + (argc-4)];
        node* root = loadBook(book);
        compress(path,root);
        freeNode(root);
    }

    LLNode** hash_table = build_hashtable(fd);
    free_hash(hash_table);
	free(hash_table);
	close(fd);
    return 0;
}
