#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

typedef struct nodes{
    char* val;
    struct node* next;
}node;

node* initNode()
{
    node* temp = malloc(sizeof(node));
    temp->next = NULL;
    temp->val = malloc(256);
}

node* insert(node* head,node* temp)
{
    node* prev = NULL;
    node* curr = head;
    while(curr!=NULL)
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

char* remSpace(char* word)
{
    char* temp = malloc(100);
    int i;
    int k = 0;
    for(i = 0;i<strlen(word)+1;i++)
    {
        if(word[i] > '!')
        {
            temp[k] = word[i];
            k++;
        }
    }
    return temp;
}

int main(int argc,char** argv)
{
    char* file = argv[2]; 
    int fd = open(file,O_RDONLY);
    char* line = malloc(257);
    read(fd,line,256);
    char* word = strtok(line,",");
    node* head = NULL;
    node* temp;
    while(word)
    {
        word = remSpace(word);
        temp = initNode();
        temp->val = word;
        head = insert(head,temp);
        word = strtok(NULL,",");
    }
    while(head!=NULL)
    {
        printf("%s\n",head->val);
        head = head->next;
    }
    return 0;
}