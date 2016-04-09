#include <linux/rotation.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/sched.h>

#define FALSE 0
#define TRUE 1
#define WRITER 0
#define READER 1

struct lock_list{
  struct rotation_range range;
  struct list_head lst;
  pid_t pid;
  int rw;
};

struct dev_rotation curr_rot;

DEFINE_SPINLOCK(rot_lock);
DEFINE_SPINLOCK(stat_lock);

struct list_head lock_waiting = LIST_HEAD_INIT(lock_waiting);
struct list_head lock_acquired = LIST_HEAD_INIT(lock_acquired);

void exit_rotlock(void) {
  struct list_head *p;
  struct list_head *n;
  struct lock_list *a;

  list_for_each_safe(p, n, &lock_waiting) {
    a = list_entry(p, struct lock_list, lst);
    list_del(p);
    kfree(a);
  }

  list_for_each_safe(p, n, &lock_acquired) {
    a = list_entry(p, struct lock_list, lst);
    list_del(p);
    kfree(a);
  }
}

int set_rotation(struct dev_rotation *rot)
{
  int cnt = 0;
  struct lock_list *a;
  struct list_head *p;
  struct list_head *n;
  int deg;
  int high;
  int low;
  struct rotation_range *temp_r;

  spin_lock(&rot_lock);
  spin_lock(&stat_lock);

  copy_from_user(&curr_rot, rot, sizeof(struct dev_rotation));

  /* TODO deg >= 360 --> ERROR! */

  list_for_each_safe(p, n, &lock_waiting){
    a = list_entry(p, struct lock_list, lst);
    temp_r = &(a->range);
    deg = temp_r->rot.degree;
    high = deg + temp_r->degree_range;
    low = deg - temp_r->degree_range;

    if( ( low <= curr_rot.degree && curr_rot.degree <= high && high < 360 && low >= 0 ) ||
        ( low < 0 && high < 360 && ( curr_rot.degree <= high || (low+360) <= curr_rot.degree ) ) ||
        ( low >= 0 && high >= 360 && ( curr_rot.degree <= (high-360) || low <= curr_rot.degree) ) ){
       cnt++;
       wake_up_process( pid_task( find_vpid(a->pid), PIDTYPE_PID)  ); /* TODO: error handling */
    }
  }

  spin_unlock(&stat_lock);
  spin_unlock(&rot_lock);

  return cnt;
}

 int rotlock_read(struct rotation_range *rot)
{
  struct rotation_range temp;
  copy_from_user(&temp, rot, sizeof(struct rotation_range));
  int low = temp.rot.degree - temp.degree_range;
  int high = temp.rot.degree + temp.degree_range;

  int distance;
  struct list_head *a;
  struct list_head *n;
  struct lock_list *p;
  int found = FALSE;

  struct lock_list *d = kmalloc(sizeof(struct lock_list), GFP_KERNEL);
  d->range = *rot;
  d->pid = current->pid;
  d->rw = READER;

  spin_lock(&stat_lock);

  /* waiting rotation*/
  list_add(&(d->lst), &lock_waiting);
  
  spin_unlock(&stat_lock);

  spin_lock(&rot_lock);

  while( !( ( low <= curr_rot.degree && curr_rot.degree <= high && high < 360 && low >= 0 ) ||
        ( low < 0 && high < 360 && ( curr_rot.degree <= high || (low+360) <= curr_rot.degree ) ) ||
        ( low >= 0 && high >= 360 && ( curr_rot.degree <= (high-360) || low <= curr_rot.degree) ) ) ) {
    spin_unlock(&rot_lock);
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    spin_lock(&rot_lock);
  }
 
  spin_unlock(&rot_lock);

  spin_lock(&stat_lock);

  /*TODO: put into the acquired queue with checking policies */

  while(1){
    printk("----------------------------looping!!!!!\n");
    found = FALSE;
    list_for_each_safe(a, n, &lock_acquired){
      p = list_entry(a, struct lock_list, lst);
      distance = p->range.rot.degree - rot->rot.degree;
      if (distance < 0) distance = -distance;
      if (distance > 180) distance = 360 - distance;
    
      if (distance <= p->range.degree_range + rot->degree_range && p->rw == WRITER){
          printk("----------------------writer here!!!!!\n");
          found = TRUE;
          break;
      }
    }
    if (found == FALSE) {
      list_for_each_safe(a, n, &lock_waiting) {
        p = list_entry(a, struct lock_list, lst);
        distance = p->range.rot.degree - rot->rot.degree;
        if (distance < 0) distance = -distance;
        if (distance > 180) distance = 360 - distance;

        if (distance <= p->range.degree_range + rot->degree_range && p->rw == WRITER) {
          printk("---------------------no lock and waiting writer here!!\n");
          found = TRUE;
          break;
        }
      }
    }
    if(found == FALSE) break;

    spin_unlock(&stat_lock);
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    spin_lock(&stat_lock);
  }

  list_del(&(d->lst));
  list_add(&(d->lst), &lock_acquired);

  spin_unlock(&stat_lock);

  return 0;
}

 int rotlock_write(struct rotation_range *rot)
{
  struct rotation_range temp;
  copy_from_user(&temp, rot, sizeof(struct rotation_range));
  int low = temp.rot.degree - temp.degree_range;
  int high = temp.rot.degree + temp.degree_range;

  int distance;
  struct list_head *a;
  struct list_head *n;
  struct lock_list *p;
  int found = FALSE;

  struct lock_list *d = kmalloc(sizeof(struct lock_list), GFP_KERNEL);
  d->range = *rot;
  d->pid = current->pid;
  d->rw = WRITER;

  spin_lock(&stat_lock);

  /* waiting rotation*/
  list_add(&(d->lst), &lock_waiting);
  
  spin_unlock(&stat_lock);

  
  spin_lock(&rot_lock);

  while( !( ( low <= curr_rot.degree && curr_rot.degree <= high && high < 360 && low >= 0 ) ||
        ( low < 0 && high < 360 && ( curr_rot.degree <= high || (low+360) <= curr_rot.degree ) ) ||
        ( low >= 0 && high >= 360 && ( curr_rot.degree <= (high-360) || low <= curr_rot.degree) ) ) ) {
    spin_unlock(&rot_lock);
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    spin_lock(&rot_lock);
  }

  spin_unlock(&rot_lock);

  spin_lock(&stat_lock);

  /* TODO: if waiting reader exists, a writer waits no matter what */

	while(1){
		found = FALSE;
		list_for_each_safe(a, n, &lock_acquired){
			p = list_entry(a, struct lock_list, lst);
			distance = p->range.rot.degree - rot->rot.degree;
			if (distance < 0) distance = -distance;
			if (distance > 180) distance = 360 - distance;

			if (distance <= p->range.degree_range + rot->degree_range){
				found = TRUE;
				break;
			}
		}
		if(found == FALSE) break;
    spin_unlock(&stat_lock);
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
    spin_lock(&stat_lock);
	}

	list_del(&(d->lst));
	list_add(&(d->lst), &lock_acquired);

	spin_unlock(&stat_lock);

	return 0;
}

/* Release a read/or write lock using the given rotation range
 * returning 0 on success, -1 on failure.
 * system call numbers 387 and 388 */
int rotunlock_read(struct rotation_range *rot)
{
  struct lock_list *a;
  struct list_head *p;
  struct list_head *n;
  int found = FALSE;

  spin_lock(&stat_lock);


  /*TODO: waking up */
  int distance;
  list_for_each_safe(p, n, &lock_waiting) {
    a = list_entry(p, struct lock_list, lst);
    distance = a->range.rot.degree - rot->rot.degree;
    if (distance < 0) distance = -distance;
    if (distance > 180) distance = 360 - distance;
    if (distance <= a->range.degree_range + rot->degree_range){
       if (a->rw == WRITER) {
         wake_up_process( pid_task( find_vpid(a->pid), PIDTYPE_PID) );  /* TODO: error */
         break;
       }
    }
  }

  list_for_each_safe(p, n, &lock_acquired) {
    a = list_entry(p, struct lock_list, lst);
    if (a->pid == current->pid && a->rw == READER) {   /* TODO: how to get the pid */
      found = TRUE;
      list_del(p);
      kfree(a);
      break;
    }
  }
  if (found == FALSE) /*error*/;

  spin_unlock(&stat_lock);

  return 0;
}

int rotunlock_write(struct rotation_range *rot)
{
  struct lock_list *a;
  struct list_head *p;
  struct list_head *n;
  int found = FALSE;
  int exist_waiting_reader = FALSE;

  spin_lock(&stat_lock);


  /* waking up */
  int distance;
  list_for_each_safe(p, n, &lock_waiting) {
    a = list_entry(p, struct lock_list, lst);
    distance = a->range.rot.degree - rot->rot.degree;
    if (distance < 0) distance = -distance;
    if (distance > 180) distance = 360 - distance;
    if (distance <= a->range.degree_range + rot->degree_range){
       if (a->rw == READER) {
         wake_up_process( pid_task( find_vpid(a->pid), PIDTYPE_PID) );  /* TODO: error */
				 exist_waiting_reader = TRUE;
       }
    }
  }

  if(exist_waiting_reader == FALSE){
    list_for_each_safe(p, n, &lock_waiting) {
      a = list_entry(p, struct lock_list, lst);
      distance = a->range.rot.degree - rot->rot.degree;
      if (distance < 0) distance = -distance;
      if (distance > 180) distance = 360 - distance;
      if (distance <= a->range.degree_range + rot->degree_range){
         if(a->rw == WRITER){
           wake_up_process( pid_task( find_vpid(a->pid), PIDTYPE_PID) );
           break;
         }
      }
    } 
  }

  list_for_each_safe(p, n, &lock_acquired){
    a = list_entry(p, struct lock_list, lst);
    if(a->pid == current->pid && a->rw == WRITER){
      found = TRUE;
      list_del(p);
      kfree(a);
      break;
    }
  }

  if(found == FALSE) /*error*/;

  spin_unlock(&stat_lock);

  return 0;
}
