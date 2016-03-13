
#include <stdio.h>
#include <malloc.h>
#include <linux/prinfo.h>
#include <linux/types.h>

#define __NR_ptree 384
#define __MAX__ 75

int main()
{
 int i, process;
 int test_nr = __MAX__;
 struct prinfo *test_buf = (struct prinfo*)malloc(sizeof(struct prinfo)*__MAX__);
 printf("Syscall started\n");
 process = syscall(__NR_ptree, test_buf, &test_nr);
 printf("Syscall is done\nNow Print tasks\n");
 printf("%s, %d, %d, %d, %d\n",test_buf[0].comm,test_buf[0].pid,test_buf[0].parent_pid,test_buf[0].first_child_pid,test_buf[0].next_sibling_pid);
 for(i=1;i< ( (process < __MAX__) ? process : __MAX__ ) ;i++)
 {
   
 printf("Name : %s, Pid: %d,Parent: %d,F.Child: %d,Next: %d\n",test_buf[i].comm,test_buf[i].pid,test_buf[i].parent_pid,test_buf[i].first_child_pid,test_buf[i].next_sibling_pid);
 }
 
 return 0;
}
