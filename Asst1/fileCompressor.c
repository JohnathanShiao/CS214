#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

typedef struct node
{
    int count;
    char* data;
    struct node* left;
    struct node* right;
}node;

typedef struct minheap
{
	int size;
	int capacity;
	node** list;
}minheap;

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
    temp->data = NULL;
    return temp;
}
node* initHeapNode(char* data, int count)
{
    node* temp = myMalloc(sizeof(node));
    temp->left = NULL;
    temp->right = NULL;
    temp->count = count;
    temp->data = data;
    return temp;
}

minheap* initMinHeap(int capacity)
{
	minheap* temp = myMalloc(sizeof(minheap));
	temp->size = 0;
	temp->capacity = capacity;
	temp->list = myMalloc(capacity*sizeof(node*));
	return temp;
}

void swap(node** n1, node** n2)
{
	node* temp = *n1;
	*n1 = *n2;
	*n2 = temp;
}

void heapify(minheap* m, int n)
{
	int min = n;
	int left = (2*n) + 1;
	int right = (2*n) + 2;
	
	if((left < m->size) && ((m->list[left]->count) < (m->list[min]->count)))
		min = left;

	if((right < m->size) && ((m->list[right]->count) < (m->list[min]->count)))
		min = right;

	if(min != n){
		swap(&m->list[min], &m->list[n]);
		heapify(m, min);
	}
}

minheap* buildMinHeap(char* data, int* count, int size) //in main need to keep count of how many tokens there are
{
	minheap* minheap = initMinHeap(size);
	int i;
	for(i = 0; i < size; i++)
	{
		minheap->list[i] = initHeapNode(data[i], count[i]);
	}
	minheap->size = size;
	
	int n = (minheap->size)-1;
	int j;
	for(j = (n-1)/2; i >= 0; --i)
		heapify(minheap, i); 

	return minheap;
}


void decompress(node* root, char* file)
{
    int fd = open(file,O_RDONLY);
    if (fd < 0)
        return;
    int len = strlen(file);
    char* fileName = myMalloc(len-4);
    memcpy(fileName,file,len-4);
    int wfd = open(fileName, O_WRONLY | O_APPEND | O_CREAT,00600);
    if (wfd < 0)
        return;
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
                write(wfd," ",1);
                ptr = root;
            }
        }
        else
        {
            ptr = ptr->right;
            if(ptr->data != NULL)
            {
                write(wfd,ptr->data,strlen(ptr->data));
                write(wfd," ",1);
                ptr = root;
            }
        }
    }
}

int main(int argc, char** argv)
{
	
    char* flag = argv[1];
    char* path = argv[2];
    node* root = initNode();
    node* ptr;
    root->count = 100;
    root->left = initNode();
    ptr = root->left;
    ptr->count = 45;
    ptr->data = "and";
    root->right = initNode();
    ptr = root->right;
    ptr->count = 55;
    ptr->left = initNode();
    ptr->right = initNode();
    ptr = ptr->left;
    ptr->count = 25;
    ptr->left = initNode();
    ptr->right = initNode();
    ptr = ptr->left;
    ptr->data = "cat";
    ptr->count = 12;
    ptr = root;
    ptr = ptr->right;
    ptr = ptr->left;
    ptr = ptr->right;
    ptr->data = "button";
    ptr->count = 13;
    ptr = root;
    ptr = ptr->right;
    ptr = ptr->right;
    ptr->left = initNode();
    ptr->right = initNode();
    ptr->count = 30;
    ptr = ptr->right;
    ptr->data = "ball";
    ptr->count = 16;
    ptr = root;
    ptr = ptr->right;
    ptr = ptr->right;
    ptr = ptr->left;
    ptr->left = initNode();
    ptr->right = initNode();
    ptr->count = 14;
    ptr = ptr->left;
    ptr->data = "a";
    ptr->count = 5;
    ptr = root;
    ptr = ptr->right;
    ptr = ptr->right;
    ptr = ptr->left;
    ptr = ptr->right;
    ptr->data = "dog";
    ptr->count = 9;
    decompress(root,path);

	char arr[] = {'a', 'b', 'c', 'd', 'e', 'f'};
	int freq[] = {5, 9, 12, 13, 16, 45};

	int size = 6;

	minheap* m = buildMinHeap(arr, freq, size);
    return 0;
}
