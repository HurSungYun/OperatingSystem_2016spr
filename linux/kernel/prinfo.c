/* headers considered to be necessary
 * 1. prinfo.h; definition of prinfo
 * 2. sched.h; definition of task_struct
 * 3. errno.h; definition of errors -> not so sure about this -> it is in uapi/asm folder
 * 4. list.h; definition of linked list and its macros
 * 5. uaccess.h; definition of access_ok
 * 6. linkage.h; definition of asmlinkage
 * 7. types.h; definition of pid_t
 * add additional headers if judged to be necessary
 */

#include <linux/prinfo.h>
#include <linux/sched.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/list.h>
#include <asm-generic/uaccess.h>
#include <linux/linkage.h>
#include <asm-generic/unistd.h>
#include <linux/types.h>


/* TODO:
 * error handling
 */
static struct prinfo task_to_info(struct task_struct *t, struct list_head sibling_head) {
  //change type task_struct into type prinfo 
  struct prinfo ret;
  struct task_struct *first_child = 
    (t->children.next == &t->children) ? NULL :
    list_entry (t->children.next, struct task_struct, sibling);
  struct task_struct *next_sibling =
    (t->sibling.next == &sibling_head) ? NULL :
    list_entry (t->sibling.next, struct task_struct, sibling);
 
  ret.state = t->state;
  ret.pid = t->pid;
  ret.parent_pid = (t->real_parent == t) ? 0 : t->real_parent->pid;
  ret.first_child_pid = (first_child == NULL) ? 0 : first_child->pid;
  ret.next_sibling_pid = (next_sibling == NULL) ? 0 : next_sibling->pid;
  ret.uid = t->real_cred->uid;
  get_task_comm(ret.comm, t); 

  return ret;
}

/* sibling_head added to the argument of DFS
 * in order to convey the information of first sibling when converting struct task_struct to struct prinfo
 */
static void DFS(struct task_struct *t, struct prinfo *buf, int size, int *nr, struct list_head sibling_head) {
  //if buffer is full return with the current buffer
  if (size == *nr) return;

  //add current task t to the buffer after changing it into struct prinfo
  //increase the number of tasks in the buffer afterwards.
  struct prinfo info = task_to_info(t, sibling_head);
  buf[*nr] = info;
  *nr += 1;

  //run DFS recursively using list macros 
  struct task_struct *child;
  struct list_head *list;
  list_for_each (list, &t->children) {
    child = list_entry (list, struct task_struct, sibling);
    DFS(child, buf, size, nr, t->children);
  }
}

static int count_task() {
  //count the total number of processes
  int ret = 0;
  struct task_struct *p;
  for_each_process (p) ret += 1;
  return ret;
}

asmlinkage int sys_ptree(struct prinfo *buf, int *nr) {
  //checking the basic error conditions
  if (buf == NULL || nr == NULL || *nr < 1) return -EINVAL;
  if (!access_ok (VERIFY_WRITE, nr, sizeof(int)) || !access_ok (VERIFY_WRITE, buf, sizeof(struct prinfo) * (*nr)))
      return -EFAULT;
  
  int size = *nr;
  *nr = 0;

  read_lock(&tasklist_lock);

  DFS(&init_task, buf, size, nr, init_task.sibling);

  read_unlock(&tasklist_lock);

  return count_task();
}


