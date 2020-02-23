#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

typedef struct node{
    char* val;
    struct node* next;
}node;

int length;

node* initNode()
{
    node* temp = malloc(sizeof(node));
    temp->next = NULL;
    return temp;
}

void freeList(node* head)
{
    node* temp;
    while(head!=NULL)
    {
        temp = head;
        head = head->next;
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
    char* w = calloc(length, sizeof(char));
    int i;
    for(i = 0;i<length;i++)
    {
        w[i] = *word->val;
        word= word->next;
    }
    node* temp = initNode();
    if(length == 0)
        temp->val = "";
    else
        temp->val = w;
    head = insert(head,temp);
    return head;
}

int comparator_int(void* n1, void* n2)
{
    int num1 = atoi(n1);
    int num2 = atoi(n2);
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
    }
    else if(len1 == 0)
        return -1;
    else if(len2 == 0)
        return 1;
    else
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

int insertionSort(void* toSort, int (*comparator)(void*, void*))
{
    node* head = (node*) toSort;
    if(head == NULL)    //empty LL
        return 0;

	node* sorted = malloc(sizeof(node));
	sorted->val = head->val;
	sorted->next = NULL;
	
    if(head->next != NULL) //more than one node
    { 
		node* ptr = NULL;
		for(ptr = head->next; ptr != NULL; ptr = ptr->next)
		{
			node* temp = malloc(sizeof(node));
			temp->val = ptr->val;
	
			if(comparator(temp->val, sorted->val) == -1)
			{
				temp->next = sorted;
				sorted = temp;
			}else
			{
				node* prev = NULL;
				node* curr = sorted;
				while(curr != NULL && (comparator(curr->val, temp->val) != 1))
				{
					prev = curr;
					curr = curr->next;
				}
				prev->next = temp;
				temp->next = curr;
			}
		}
	}
	node* print = NULL;
    for(print = sorted; print != NULL; print = print->next)
    {
        printf("%s\n", print->val);
    }	
    freeList(head);
    freeList(sorted);
    return 1;
}
int quickSort(void* toSort, int(*comparator)(void*,void*))
{
    node* pivot = (node*)toSort;
    if(pivot == NULL)
        return 0;
    node* ptr = pivot->next;
    node* lHead;
    node* rHead;
    node* temp;
    lHead = rHead = temp = NULL;
    while(ptr!=NULL)
    {
        if(comparator(ptr->val,pivot->val) < 0)
        {
            temp = initNode();
            memcpy(temp,ptr,sizeof(node));
            temp->next = NULL;
            lHead = insert(lHead,temp);
        }
        else
        {
            temp = initNode();
            memcpy(temp,ptr,sizeof(node));
            temp->next = NULL;
            rHead = insert(rHead,temp);
        }
        node* prev = ptr;
        ptr = ptr->next;
        free(prev);
    }
    if(!quickSort(lHead,comparator))
    {
        printf("%s\n",pivot->val);
        free(pivot);
    }
    quickSort(rHead,comparator);
}

int main(int argc,char** argv)
{
    int sortType;
    char* file = argv[2];   
    char* sort = argv[1];
    if(comparator_string(sort, "-q") == 0)
        sortType = 1;
    else if(comparator_string(sort, "-i") == 0)
        sortType = 0;
    else
    {
        printf("Fatal Error %s is not a valid flag",argv[1]);
        return 0;
    }
    int fd = open(file,O_RDONLY);
    char* c = malloc(sizeof(char));
    node* head = NULL;                  //linked list of chars to create a token
    node* temp;
    node* list = NULL;                  //linked list of tokens from file
    length = 0;
    while(read(fd,c,1) > 0)
    {
        if(*c != ',')
        {
            if(*c != ' ' && *c != '\n' && *c != '\t')
            {
                length+=1;
                temp = initNode();
                temp->val = calloc(1, sizeof(char));
                memcpy(temp->val,c,1);
                head = insert(head,temp);
            }
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
    int cmpType = atoi(list->val);
    if(sortType == 1)
    {
        if(cmpType != 0)
            quickSort(list,comparator_int);
        else
            quickSort(list,comparator_string);
    }else
    {
        if(cmpType != 0)
            insertionSort(list, comparator_int);
        else    
            insertionSort(list, comparator_string);
    }
    close(fd);
    return 0;
}
