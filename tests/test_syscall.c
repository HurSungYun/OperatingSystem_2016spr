#include<stdio.h>
#include<linux/unisted.h>

int main()
{
 printf("Syscall started\n");
 printf("%d\n",syscall(__NR_ptree));
 printf("Syscall ended\n");
 return 0;
}
