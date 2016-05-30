#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/gps.h>
#include <linux/spinlock.h>

struct gps_location curr;
EXPORT_SYMBOL(curr);
DEFINE_SPINLOCK(loc_lock);
EXPORT_SYMBOL(loc_lock);

int set_gps_location(struct gps_location __user *loc)
{
	spin_lock(&loc_lock);
	copy_from_user(&curr, loc, sizeof(struct gps_location));
	spin_unlock(&loc_lock);
	return 0;
}
