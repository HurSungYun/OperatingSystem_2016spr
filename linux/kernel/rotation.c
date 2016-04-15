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

DEFINE_SPINLOCK(one_lock);

struct list_head lock_waiting = LIST_HEAD_INIT(lock_waiting);
struct list_head lock_acquired = LIST_HEAD_INIT(lock_acquired);

void exit_rotlock(void) {
  struct list_head *p;
  struct list_head *n;
  struct lock_list *a;

  spin_lock(&one_lock);
  list_for_each_safe(p, n, &lock_waiting) {
    a = list_entry(p, struct lock_list, lst);
    if (a->pid == current->pid) {
      list_del(p);
      kfree(a);
    }
  }

  list_for_each_safe(p, n, &lock_acquired) {
    a = list_entry(p, struct lock_list, lst);
    if (a->pid == current->pid) {
      list_del(p);
      kfree(a);
    }
  }
  spin_unlock(&one_lock);
}

int set_rotation(struct dev_rotation *rot)
{
  int cnt = 0;
  struct lock_list *a;
  struct list_head *p;
  struct list_head *n;
  int deg;
  int distance;
  struct rotation_range *temp_r;
  int iswriter = FALSE;
  int isreader = FALSE;
  int reader_count = 0;

  if (rot->degree < 0 || rot->degree >= 360)
    return -EINVAL;

  spin_lock(&one_lock);

  copy_from_user(&curr_rot, rot, sizeof(struct dev_rotation));

  list_for_each_safe(p, n, &lock_acquired) {
    a = list_entry(p, struct lock_list, lst);
    temp_r = &(a->range);
    deg = temp_r->rot.degree;
    distance = curr_rot.degree - deg;
    if (distance < 0) distance = - distance;
    if (distance > 180) distance = 360 - distance;
    if (distance <= temp_r->degree_range && a->rw == WRITER) {
      iswriter = TRUE;
      break;
    }
    else if (distance <= temp_r->degree_range && a->rw == READER) {
      isreader = TRUE;
      break;
    }
  }

  if (iswriter == TRUE) {
    spin_unlock(&one_lock);
    return 0;
  }
  else if (isreader == TRUE) {
    list_for_each_safe(p, n, &lock_waiting){
      a = list_entry(p, struct lock_list, lst);
      temp_r = &(a->range);
      deg = temp_r->rot.degree;
      distance = curr_rot.degree - deg;
      if (distance < 0) distance = - distance;
      if (distance > 180) distance = 360 - distance;
      if (distance <= temp_r->degree_range && a->rw == WRITER) {
        spin_unlock(&one_lock);
        return 0;
      }
      else if (distance <=temp_r->degree_range && a->rw == READER) {
				struct lock_list *cr, *cr_tmp;
				int dist_wr_aw;
				int overlapping;
				overlapping = FALSE;
        list_for_each_entry_safe(cr, cr_tmp, &lock_waiting, lst){
					/*check if there is a overlapping writer holding the lock*/
					if (cr->rw == READER) continue;
					dist_wr_aw = cr->range.rot.degree - deg;
					if (dist_wr_aw < 0) dist_wr_aw = - dist_wr_aw;
					if (dist_wr_aw > 180) dist_wr_aw = 360 - dist_wr_aw;
					if (dist_wr_aw <= temp_r->degree_range + cr->range.degree_range){ 
						overlapping = TRUE;
						break;
					}
				}
				if (overlapping == FALSE){	
					wake_up_process( pid_task( find_vpid(a->pid), PIDTYPE_PID)  );
					reader_count ++;
				}
      }
    }
    spin_unlock(&one_lock);
    return reader_count;
  }
  else {
    list_for_each_safe(p, n, &lock_waiting){
      a = list_entry(p, struct lock_list, lst);
      temp_r = &(a->range);
      deg = temp_r->rot.degree;
      distance = curr_rot.degree - deg;
      if (distance < 0) distance = - distance;
      if (distance > 180) distance = 360 - distance;
      if (distance <= temp_r->degree_range && a->rw == WRITER) {
        wake_up_process( pid_task( find_vpid(a->pid), PIDTYPE_PID)  );
        spin_unlock(&one_lock);
        return 1;
      }
      else if (distance <= temp_r->degree_range && a->rw == READER) {
        wake_up_process( pid_task( find_vpid(a->pid), PIDTYPE_PID)  );
        reader_count ++;
      }
    }
    spin_unlock(&one_lock);
    return reader_count;
  }
}

int rotlock_read(struct rotation_range *rot)
{
  struct rotation_range temp;
  copy_from_user(&temp, rot, sizeof(struct rotation_range));
  if (temp.rot.degree < 0 || temp.rot.degree >= 360) return -EINVAL;
  if (temp.degree_range >= 180) return -EINVAL;

  int distance;
  struct list_head *a;
  struct list_head *n;
  struct lock_list *p;
  int found = FALSE;
  int distance_2;
  int distance_curr;

  struct lock_list *d = kmalloc(sizeof(struct lock_list), GFP_KERNEL);
  if (d == NULL) return -ENOMEM;
  d->range = temp;
  d->pid = current->pid;
  d->rw = READER;


  spin_lock(&one_lock);

  /* waiting rotation*/
  list_add_tail(&(d->lst), &lock_waiting);

  while(1){
    found = FALSE;
    list_for_each_safe(a, n, &lock_acquired){
      p = list_entry(a, struct lock_list, lst);
      distance = p->range.rot.degree - temp.rot.degree;
      if (distance < 0) distance = -distance;
      if (distance > 180) distance = 360 - distance;

      if (distance <= p->range.degree_range + temp.degree_range && p->rw == WRITER){
        /*writer currently in overlapping area*/
        found = TRUE;
        break;
      }
    }
    if (found == FALSE) {
      list_for_each_safe(a, n, &lock_waiting) {
        p = list_entry(a, struct lock_list, lst);
        distance = p->range.rot.degree - temp.rot.degree;
        distance_2 = p->range.rot.degree - curr_rot.degree;
        if (distance < 0) distance = -distance;
        if (distance > 180) distance = 360 - distance;
        if (distance_2 < 0) distance_2 = - distance_2;
        if (distance_2 > 180) distance_2 = 360 - distance_2;

        if (distance <= p->range.degree_range + temp.degree_range 
            && distance_2 <= p->range.degree_range
            && p->rw == WRITER) {
          found = TRUE;
          break;
        }
      }
    }

		distance_curr = temp.rot.degree - curr_rot.degree;
    if (distance_curr < 0) distance_curr = - distance_curr;
    if (distance_curr > 180) distance_curr = 360 - distance_curr;

    if(found == FALSE && distance_curr <= temp.degree_range) break;

    spin_unlock(&one_lock);
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    spin_lock(&one_lock);
  }

  list_del_init(&(d->lst));
  list_add_tail(&(d->lst), &lock_acquired);

  spin_unlock(&one_lock);

  return 0;
}

int rotlock_write(struct rotation_range *rot)
{
  struct rotation_range temp;
  copy_from_user(&temp, rot, sizeof(struct rotation_range));
  if (temp.rot.degree < 0 || temp.rot.degree >= 360) return -EINVAL;
  if (temp.degree_range >= 180) return -EINVAL;

  int distance;
  struct list_head *a;
  struct list_head *n;
  struct lock_list *p;
  int found = FALSE;
  int distance_curr;

  struct lock_list *d = kmalloc(sizeof(struct lock_list), GFP_KERNEL);
  if (d == NULL) return -ENOMEM;
  d->range = temp;
  d->pid = current->pid;
  d->rw = WRITER;


  spin_lock(&one_lock);

  /* waiting rotation*/
  list_add_tail(&(d->lst), &lock_waiting);

  while(1){
    found = FALSE;
    list_for_each_safe(a, n, &lock_acquired){
      p = list_entry(a, struct lock_list, lst);
      distance = p->range.rot.degree - temp.rot.degree;
      if (distance < 0) distance = -distance;
      if (distance > 180) distance = 360 - distance;

      if (distance <= p->range.degree_range + temp.degree_range){
        found = TRUE;
        break;
      }
    }
    distance_curr = temp.rot.degree - curr_rot.degree;
    if (distance_curr < 0) distance_curr = - distance_curr;
    if (distance_curr > 180) distance_curr = 360 - distance_curr;

    if(found == FALSE && distance_curr <= temp.degree_range) break;

    spin_unlock(&one_lock);
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    spin_lock(&one_lock);
  }

  list_del_init(&(d->lst));
  list_add_tail(&(d->lst), &lock_acquired);

  spin_unlock(&one_lock);

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
  struct rotation_range temp;
  copy_from_user(&temp, rot, sizeof(struct rotation_range));
  if (temp.rot.degree < 0 || temp.rot.degree >= 360) return -EINVAL;
  if (temp.degree_range >= 180) return -EINVAL;

  spin_lock(&one_lock);

  int distance;
	int distance_curr;

  list_for_each_safe(p, n, &lock_acquired) {
    a = list_entry(p, struct lock_list, lst);
    if (a->pid == current->pid && a->rw == READER && 
        temp.degree_range == a->range.degree_range && temp.rot.degree == a->range.rot.degree) { 
      found = TRUE;
      list_del(p);
      kfree(a);
      break;
    }
  }
  if (found == FALSE) /*error*/;

  list_for_each_safe(p, n, &lock_waiting) {
    a = list_entry(p, struct lock_list, lst);

    distance = a->range.rot.degree - temp.rot.degree;
    if (distance < 0) distance = -distance;
    if (distance > 180) distance = 360 - distance;

    distance_curr = temp.rot.degree - curr_rot.degree;
    if (distance_curr < 0) distance_curr = - distance_curr;
    if (distance_curr > 180) distance_curr = 360 - distance_curr;

    if (distance <= a->range.degree_range + temp.degree_range && distance_curr <= temp.degree_range){
      if (a->rw == WRITER) {
        wake_up_process( pid_task( find_vpid(a->pid), PIDTYPE_PID) );
        break;
      }
    }
  }


  spin_unlock(&one_lock);

  return 0;
}

int rotunlock_write(struct rotation_range *rot)
{
  struct lock_list *a;
  struct list_head *p;
  struct list_head *n;
  int found = FALSE;
  struct rotation_range temp;
  copy_from_user(&temp, rot, sizeof(struct rotation_range));
  if (temp.rot.degree < 0 || temp.rot.degree >= 360) return -EINVAL;
  if (temp.degree_range >= 180) return -EINVAL;

  spin_lock(&one_lock);

  /* waking up */
  int distance;
  int distance_curr;

	list_for_each_safe(p, n, &lock_acquired){
    a = list_entry(p, struct lock_list, lst);
		if(a->pid == current->pid && a->rw == WRITER &&
				temp.degree_range == a->range.degree_range && temp.rot.degree == a->range.rot.degree) { 
			found = TRUE;
			list_del(p);
			kfree(a);
			break;
		}
	}
 
  if(found == FALSE) /*error*/;

  list_for_each_safe(p, n, &lock_waiting) {
    a = list_entry(p, struct lock_list, lst);

    distance = a->range.rot.degree - temp.rot.degree;
    if (distance < 0) distance = -distance;
    if (distance > 180) distance = 360 - distance;

		distance_curr = temp.rot.degree - curr_rot.degree;
    if (distance_curr < 0) distance_curr = - distance_curr;
    if (distance_curr > 180) distance_curr = 360 - distance_curr;

    if (distance <= a->range.degree_range + temp.degree_range && distance_curr <= temp.degree_range){
        wake_up_process( pid_task( find_vpid(a->pid), PIDTYPE_PID) );
    }
  }
  

  spin_unlock(&one_lock);

  return 0;
}
