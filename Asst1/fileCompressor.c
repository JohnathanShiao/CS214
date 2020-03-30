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
    temp->data = myMalloc(64);
    return temp;
}

void freeList(list* head)
{
    list* temp;
    while(head!=NULL)
    {
        temp = head;
        head = head->next;
        if(strlen(temp->data) != 0)
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
    int wfd = open(fileName, O_WRONLY | O_APPEND | O_CREAT,00644);
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

int main(int argc, char** argv)
{
    if(argc < 2 || argc > 5)
    {
        printf("Error: Expected 2-4 arguments, received %d",argc);
        return 0;
    }
    if(argc == 5)
        recursive = 1;
    else
        recursive = 0;
    char* flag;
    if(strcmp(argv[1],"-R")==0)
        flag = argv[2];
    else
        flag = argv[1];
    char* path = argv[2 + (argc-4)];
    if(strcmp(flag,"-b") == 0)
    {
        char* book = argv[3 + (argc-4)];
        node* root = loadBook(book);
        decompress(root,path);
        freeNode(root);
    }
    return 0;
}