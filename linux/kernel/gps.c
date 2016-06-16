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
	if (copy_from_user(&curr, loc, sizeof(struct gps_location)))
		return -EINVAL;
	printk("set_gps_location copy done\n");
	spin_unlock(&loc_lock);
	return 0;
}

int get_gps_location(const char __user *pathname, struct gps_location __user *loc)
{
	struct inode *inode;
	struct path path;
	struct gps_location *kern_loc = kmalloc(sizeof(struct gps_location), GFP_KERNEL);
	if (kern_loc == NULL) return -ENOMEM;

	kern_path(pathname, LOOKUP_FOLLOW, &path);
	if (path.dentry == NULL)
		return -EINVAL;
	inode = path.dentry->d_inode;
	if (inode == NULL)
		return -EINVAL;

	if (inode->i_op->get_gps_location == NULL)
		return -ENODEV;
	inode->i_op->get_gps_location(inode, kern_loc);

	if (copy_to_user(loc, kern_loc, sizeof(struct gps_location)))
		return -EINVAL;
	kfree(kern_loc);
	return 0;
}
