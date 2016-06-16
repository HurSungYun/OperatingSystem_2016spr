project 4: Geo-tagged file system (ext2)
========================================

# High-level design and Implementation

## Two system calls

* Two system calls, set\_gps\_location and get\_gps\_location, each gets and sets the gps value defined as a global variable inside the kernel.

* A spinlock loc\_lock is declared to properly synchronize access to gps values of files and current gps value. It is used when accessing the global gps variable curr, or gps value of any files (Not that efficient, but safe).

* Inside get\_gps\_location, the newly defined inode operation get\_gps\_location for the file is called.

## Change in inode structure

* ext2\_inode\_info is the inode type used for memory. i\_latitude, i\_longitude (\_\_u64), and i\_accuracy (\_\_u32) are added to the structure.

* ext2\_inode is the inode type used for disk. For this inode, each component is added as a little endian type to specify the endian-ness when writing to the disk.

## New inode operations

* Two inode operations, set\_gps\_location and get\_gps\_location, are used to set or get the gps value of a specific file. They are added to the inode\_operations structure.

* The actual implementation of the functions is done in fs/ext2/inode.c.

* ext2\_set\_gps\_location copies curr to the gps components inside the file using memcpy for each component (latitude, longitude, accuracy). Similarly, ext2\_get\_gps\_location also uses memcpy to copy the gps data in the file to gps\_location pointer typed parameter.

* These functions are registered in file and directory inode operations, found in fs/ext2/file.c and fs/ext2/namei.c.

* Both new operations use the spinlock loc\_lock to safely access the gps values of every file.

* Inode in disk is moved to memory by ext2\_iget function in fs/ext2/inode.c. And it is again written back to disk by \_\_ext2\_write\_inode function. These two functions are modified to get and write the gps variables from and to disk. lexx\_to\_cpu cpu\_to\_lexx macros are used to deal with endian-ness.

## Update gps value when file/dir created/modified

* set\_gps\_location inode operation is used to set the gps value of the file whenever necessary.

* In fs/ext2/namei.c, set\_gps\_location operation is called in ext2\_create, ext2\_mkdir, ext2\_rename. Additionally, it is also called in generic\_aio\_write in mm/filemap.c to update the gps data whenever a file is modified.

## Create a file system

* Two inode structures are modified in e2fsprogs/lib/ext2fs/ext2\_fs.h to support the modified file system.

## Access Control

* ext2\_permission function is defined in fs/ext2/inode.c to control access to files and directories depending on the gps data.

* The function is linked to .permission in file and directory inode operations.

<!--- TODO: permission function explanation -->

## User space testing

* Two test codes are created (gpsupdate, file\_loc) to test the modified ext2 file system. gpsupdate round robins through randomly created gps values and sets the kernel gps value to the value. file\_loc receives one standard input (file path), and gets the gps value of the file.

<!--- TODO: more about test codes -->

# Lessons learned

* SUNG-YUN HUR
* EUN-HYANG KIM
* YEON-WOO KIM: We can easily modify the existing file system to support special operations. Given more time to think about the design, we might be able to come up with a unique file system with operations that can be widely used.
