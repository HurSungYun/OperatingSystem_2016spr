#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/gps.h>

int set_gps_location(struct gps_location __user *loc)
{
	return 0;
}
