#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/fs.h>


struct gps_location curr;
EXPORT_SYMBOL(curr);
DEFINE_SPINLOCK(loc_lock);
EXPORT_SYMBOL(loc_lock);

int set_gps_location(struct gps_location __user *loc)
{
	printk("set_gps_location started\n");
	spin_lock(&loc_lock);
	copy_from_user(&curr, loc, sizeof(struct gps_location));
	printk("set_gps_location copy done\n");
	spin_unlock(&loc_lock);
	return 0;
}

int get_gps_location(const char __user *pathname, struct gps_location __user *loc)
{
	/* TODO: return -ENODEV if no GPS coordinates are embedded in the file */
	struct inode *inode;
	struct path path;
	struct gps_location *kern_loc = kmalloc(sizeof(struct gps_location), GFP_KERNEL);
	if (kern_loc == NULL) return -ENOMEM;

	kern_path(pathname, LOOKUP_FOLLOW, &path);
	inode = path.dentry->d_inode;
	inode->i_op->get_gps_location(inode, kern_loc);

	copy_to_user(loc, kern_loc, sizeof(struct gps_location));
	kfree(kern_loc);
	return 0;
}
