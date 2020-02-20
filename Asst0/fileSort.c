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

int length;

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
    char* w = malloc(sizeof(char) * length);
    int i;
    for(i = 0;i<length;i++)
    {
        w[i] = *word->val;
        word= word->next;
    }
    token* temp = initToken();
    if(length == 0)
        temp->val = "";
    else
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
int comparator_int(void* n1, void* n2)
{
    int num1 = *(int*) n1;
    int num2 = *(int*) n2;
    if(num1 > num2)
        return 1;
    else if(num1 < num2)
        return -1;
    return 0; //if num1 = num2
}

int comparator_string(void* s1, void* s2)
{
    char* str1 = (char*) s1;
    char* str2 = (char*) s2;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int count = 0;
    int flag = -1;
    if(len1 > len2)
    {
        count = len2;
        flag = 0;
    }else if(len1 < len2)
    {
        count = len1;
        flag = 1;
    }else
        count = len1;
    int i;
    for(i = 0; i < count; i++)
    {
        if(str1[i] < str2[i])
            return -1;
        else if(str1[i] > str2[i])
            return 1;
    }
    if(flag != -1)
    {
        if(flag == 0)
            return 1;
        else
            return -1;
    }
    return 0;
}

int main(int argc,char** argv)
{
    char* file = argv[2];
    int fd = open(file,O_RDONLY);
    char* c = malloc(sizeof(char));
    node* head = NULL;
    node* temp;
    token* list = NULL;
    length = 0;
    while(read(fd,c,1))
    {
        if(*c != ',')
        {
            if(*c != ' ' && *c != '\n' && *c != '\t')
            {
                length+=1;
                temp = initNode();
                memcpy(temp->val,c,1);
                head = insertChar(head,temp);
            }
        }
        else
        {
            list = addToList(list,head);
            length=0;
            head = NULL;
        }
    }
    if(length >0)
        list = addToList(list,head);
    // token* it = list;
    // while(it!=NULL)
    // {
    //     printf("%s\n",it->val);
    //     it=it->next;
    // }
    return 0;
}
