
#include <stdio.h>
#include <malloc.h>
#include <linux/prinfo.h>
#include <linux/types.h>

#define __NR_ptree 384

int main()
{
 int test_nr = 64;
 struct prinfo *test_buf = (struct prinfo*)malloc(sizeof(struct prinfo)*64);
 printf("Syscall started\n");
 printf("%d\n",syscall(__NR_ptree, test_buf, &test_nr));
 //printf("%d\n",syscall(__NR_ptree, NULL, NULL));
 printf("Syscall ended\n");
 printf("Root : %d Process ID : %d child ID: %d Process_Name : %s\n", test_buf->state, test_buf->pid, test_buf->first_child_pid, test_buf->comm);
 
 return 0;
}
