#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

typedef struct node{
    char* val;
    struct node* next;
}node;

typedef struct token{
    char* val;
    struct token* next;
}token;

node* initNode()
{
    node* temp = malloc(sizeof(node));
    temp->val = malloc(sizeof(char));
    temp->next = NULL;
    return temp;
}

token* initToken()
{
    token* temp = malloc(sizeof(token));
    temp->next = NULL;
    return temp;
}

token* insertToken(token* head, token* temp)
{
    token* curr = head;
    token* prev = NULL;
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

token* addToList(token* head,node* word)
{
    node* counter = word;
    int count=0;
    while(counter!=NULL)
    {
        count++;
        counter= counter->next;
    }
    char* w = malloc(sizeof(char) * count);
    int i;
    for(i = 0;i<count;i++)
    {
        w[i] = *word->val;
        word= word->next;
    }
    token* temp = initToken();
    temp->val = w;
    head = insertToken(head,temp);
}

node* insertChar(node* head, node* temp)
{
    node* curr = head;
    node* prev = NULL;
    while(curr !=NULL)
    {
        prev = curr;
        curr = curr->next;
    }
    if(prev == NULL)
        head = temp;
    else
        prev->next = temp;
    return head;
}

int main(int argc,char** argv)
{
    char* file = argv[2];
    int fd = open(file,O_RDONLY);
    char* c = malloc(sizeof(char));
    node* head = NULL;
    node* temp;
    token* list = NULL;
    while(read(fd,c,1))
    {
        if(*c != ',')
        {
            if(*c != ' ' && *c != '\n' && *c != '\t')
            {
                temp = initNode();
                memcpy(temp->val,c,1);
                head = insertChar(head,temp);
            }
        }
        else
        {
            list = addToList(list,head);
            head = NULL;
        }
    }
    list = addToList(list,head);
    token* it = list;
    while(it!=NULL)
    {
        printf("%s\n",it->val);
        it=it->next;
    }
    return 0;
}