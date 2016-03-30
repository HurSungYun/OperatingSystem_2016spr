#include <linux/rotation.h>

int set_rotation(struct dev_rotation *rot)
{
	return 384;
}

/* Take a read/or write lock using the given rotation range
 * returning 0 on success, -1 on failure.
 * system call numbers 385 and 386
 */
 int rotlock_read(struct rotation_range *rot)
{
	return 385;
}

 int rotlock_write(struct rotation_range *rot)
{
	return 386;
}

/* Release a read/or write lock using the given rotation range
 * returning 0 on success, -1 on failure.
 * system call numbers 387 and 388 */
int rotunlock_read(struct rotation_range *rot)
{
	return 387;
}

int rotunlock_write(struct rotation_range *rot)
{
	return 388;
}
