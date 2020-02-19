#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

typedef struct cnode{
    char* val;
    struct cnode* next;
}cnode;

typedef struct node{
    char* val;
    struct node* next;
}node;

cnode* initCNode()
{
    cnode* temp = malloc(sizeof(cnode));
    temp->val = malloc(sizeof(char));
    temp->next = NULL;
    return temp;
}

cnode* insertChar(cnode* head, cnode* temp)
{
    cnode* curr = head;
    cnode* prev = NULL;
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
    cnode* head = NULL;
    cnode* temp;
    while(read(fd,c,1))
    {
        if(*c != ',')
        {
            if(*c != ' ' && *c != '\n' && *c != '\t')
            {
                temp = initCNode();
                memcpy(temp->val,c,1);
                head = insertChar(head,temp);
            }
        }
        else
        {
            while(head!=NULL)
            {
                printf("%c",*head->val);
                head=head->next;
            }
            printf("\n");
            head = NULL;
        }
    }
    return 0;
}