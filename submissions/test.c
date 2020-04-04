#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void convert(char* str)
{
    int i, count;
    count = 1;
    int size = 8 * strlen(str);
    int result = 1;
    for(i = 0;i<strlen(str)-1;i++)
    {
        int val = (int)str[i];
        while(val>0)
        {
            if(val % 2)
                result+=1;
            else
                result = result<<1;
            count++;
            val = val/2;
        }
    }
    printf("%d",result);
}

int main(int argc, char** argv)
{
    char b[] = "abcde";
    convert("jack");
    return 0;
}