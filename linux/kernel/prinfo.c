/* headers considered to be necessary
 * 1. prinfo.h; definition of prinfo
 * 2. sched.h; definition of task_struct
 * 3. errno.h; definition of errors -> not so sure about this
 * 4. list.h; definition of linked list and its macros
 * 5. uaccess.h; definition of access_ok
 * add additional headers if judged to be necessary
 */
#include <linux/prinfo.h>
#include <linux/sched.h>
#include <asm/errno.h>
#include <linux/list.h>
#include <asm/uaccess.h>

//function prototype
int ptree(struct prinfo *buf, int *nr);
static void DFS(struct task_struct *t, struct prinfo *buf, int size, int *nr);
static struct prinfo task_to_info(struct task_struct *t);



/* TODO:
 * error handling
 * setting pid to 0 when no parent || no children || no next sibling
 * make ptree return number of successes
 * system call number (384) assignment
 */



int ptree(struct prinfo *buf, int *nr) {
  //checking the basic error conditions
  if (buf == NULL || nr == NULL || *nr < 1) return -EINVAL;
  if (!access_ok (VERIFY_WRITE, nr, sizeof(int) || !access_ok (VERIFY_WRITE, buf, sizeof(prinfo) * (*nr)))
      return -EFAULT;
  
  int size = *nr;
  *nr = 0;

  read_lock(&tasklist_lock);

  DFS(&init_task, buf, size, nr);

  read_unlock(&tasklist_lock);
}

static void DFS(struct task_struct *t, struct prinfo *buf, int size, int *nr) {
  //if buffer is full return with the current buffer
  if (size == *nr) return;

  //add current task t to the buffer after changing it into struct prinfo
  //increase the number of tasks in the buffer afterwards.
  struct prinfo info = task_to_info(t);
  buf[*nr] = info;
  *nr += 1;

  //run DFS recursively using list macros 
  struct task_struct *child;
  struct list_head *list;
  list_for_each (list, t->children) {
    child = list_entry (list, struct task_struct, sibling);
    DFS(child, buf, size, nr);
  }
}

static struct prinfo task_to_info(struct task_struct *t) {
  //change type task_struct into type prinfo 
  struct prinfo ret;
  struct task_struct *first_child = 
    list_entry (t->children->next, struct task_struct, sibling);
 
  ret.state = t->state;
  ret.pid = t->pid;
  ret.parent_pid = t->parent->pid;
  ret.first_child_pid = first_child->pid;
  ret.next_sibling_pid = t->sibling->next->pid;
  ret.uid = t->cred->uid;
  ret.comm = t->comm;

  return ret;
}
