#include <stdio.h>
#include <malloc.h>
#include <linux/prinfo.h>
#include <linux/types.h>

#define __NR_ptree 384
#define __MAX__ 300

int main()
{
 int i,j, process;
 int test_nr = __MAX__;
 struct prinfo *test_buf = (struct prinfo*)malloc(sizeof(struct prinfo)*__MAX__);
 printf("Syscall started\n");
 process = syscall(__NR_ptree, test_buf, &test_nr);
 printf ("Total number of entries: %d\n", process);
 //checking the total number of entries before looping through the buffer.
 printf("Syscall is done\nNow Print tasks\n");
 
 int stack[__MAX__][4],stackPointer=0,level=-1;
 
 stack[0][0]=test_buf[0].pid;
 stack[0][1]=test_buf[0].parent_pid;
 stack[0][2]=test_buf[0].first_child_pid;
 stack[0][3]=test_buf[0].next_sibling_pid;
 stackPointer++;

	for(i=1;i< ( (process < __MAX__) ? (process+1) : __MAX__ ) ;i++)
 	{
    if(stack[stackPointer-1][0]==test_buf[i].parent_pid && stack[stackPointer-1][2]==test_buf[i].pid){
      //meet the oldest child
			level++;  //increase the number of tabs
    }
    if(test_buf[i].first_child_pid!=0){
			//if the process has a child, push it in the stack
         stack[stackPointer][0]=test_buf[i].pid;
         stack[stackPointer][1]=test_buf[i].parent_pid;
         stack[stackPointer][2]=test_buf[i].first_child_pid;
         stack[stackPointer][3]=test_buf[i].next_sibling_pid;
         stackPointer++;
    }

		//print the process
    for(j=0;j<level;j++) printf("\t");
//    printf("Name : %s, Pid: %d,Parent: %d,F.Child: %d,Next: %d\n",test_buf[i].comm,test_buf[i].pid,test_buf[i].parent_pid,test_buf[i].first_child_pid,test_buf[i].next_sibling_pid);
    printf("%s,%d,%ld,%d,%d,%d,%d\n",test_buf[i].comm,test_buf[i].pid,test_buf[i].state,test_buf[i].parent_pid,test_buf[i].first_child_pid,test_buf[i].next_sibling_pid, test_buf[i].uid);

   int t[2];
   t[0]=test_buf[i].parent_pid;
   t[1]=test_buf[i].next_sibling_pid;

	 //if the process has no more siblings, pop the parent and decrease the number of tabs
   while(stack[stackPointer-1][0]==t[0] && t[1]==0){
       t[0] = stack[stackPointer-1][1];
       t[1] = stack[stackPointer-1][3];
       level--;
       stackPointer--;
     }
 }
 
 return 0;
}
