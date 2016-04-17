# <Project 2> by os-team1

## Contents
1. How to build
2. Policy
3. High level design
4. Implementation
5. Lessons learned

# 1. How to build the kernel
 
 * Build the revised kernel and push it into your device. Enter the linux/ directory and type
 
 
 >  ./build.sh tizen_tm1 USR
 
 >  sdb -d root on
 
 >  sdb push system_kernel.tar /opt/storage/sdcard/tizen-recovery.tar
 
 >  sdb shell reboot recovery
 
 * Reboot your device with the kernel.
 
 * Afterwards, compile the test code and also push it into your device. Test codes are in selector/ and trial/ forder. Enter either folder and type
 
 >  make
 
 >  sdb push <selector or trial> /home/developer/
 
 * Test
 
 >  sdb shell

 > <execute test programs in various ways> ./selector 1 & ./trial & ./trial


 # 2. Policy

We set our policy and it is stated below. 

* **Currently reader(s) grabbing the lock and release**
    + only readers waiting -> go get the lock
    + only writers waiting-> wait until all the readers unlock and one of them gets the lock
    + readers & writers waiting -> wait until current readers unlock and one of the writers gets the lock (readers wait & starvation-free)

* **Currently a writer grabbing the lock and release**
    + only readers waiting -> wait until the writer unlocks and get the lock
    + only writers waiting -> wait until the writer unlocks and one of them gets the lock
    + readers & writers waiting -> wait until the writer unlocks and one of the writers gets the lock (our team's own policy)

* **Currently no one grabbing the lock**
    + only readers waiting-> go get the lock
    + only writers waiting -> one of them gets the lock
    + readers & writers waiting -> one of the writers gets the lock (our team's own policy)

 
 # 3. High level Design

We have set 5 system calls and it interacts each other. The statement below is how they work.

 * set_rotation()
    + set\_rotation() is system call function we set. It changes current device rotation. Also, it wakes up all the appropriate tasks considering all cases and return the number of tasks which get the lock. 

 * rotlock_read()
    + when rotlock\_read() is called, it checks whether there is a lock-acquired writer which is overlapped with target. If so, make current task sleep. If not, checks whether there is a starving writer lock and whether current degree is in range, and gives a lock if there is no starving writer lock. Otherwise, sleep the task as well.

 * rotlock_write()
    + rotlock\_write() is simpler than rotlock\_read(). It checks whether there is a lock which is overlapped with target and whether current degree is in range and gives a lock if there isn't any acquired lock overlapping with target. Otherwise sleep.

 * rotunlock_read()
    + rotunlock\_read() consists of two features. The first one is releasing the lock it had before. The second one is re-awaking tasks for liveness. We check whether there is a starving writer on waiting list, and wake it up if there exists.

 * rotunlock_write()
    + rotunlock\_write() consists of two feature as well. The first one is same as rotlock\_read(). The second feature is a little bit different. It broadcasts all waiting tasks and there would be resulted correctly because of other functions. If there is a waiting writer lock, it would grab the lock, and if there's no waiting writer lock, readers grab locks.

 
 # 4. Implementation
 
We implemented only 6 functions. 5 functions are syscall functions and the other one is exit/_rotlock().

 * exit_rotlock()
    + exit\_rotlock() is used for correctness when exit. It is called when do\_exit() is processed in linux kernel. Therefore, it can prevent error at the next execution. 

We used one global lock and it is spinlock. Also, We have two lists called "lock\_waiting" and "lock\_acquired". They store list of waiting tasks and acquired tasks each. The list is called lock\_list in our code and its definition is

```
struct lock_list{
  struct rotation_range range;
  struct list_head lst;
  pid_t pid;
  int rw;
};
```

and they are dynamically allocated. We iterate these lists when we need to manipulate it. 


Additionally, we check overlapping areas using this method.

* (distance between the center of two range) <= (sum of range_degree of two)


 # 5. Lessons learned
 
 * HURSUNGYUN (2014-19768): I absolutely found that writing concurrent program with correctness is one of the most difficult tasks in the world.
 
 * YEONWOOKIM (2014-17184): 
 
 * EUNHYANGKIM (2013-13494): I absolutely found that if we are stuck in one algorithm, it's way better to abort it without any hesitation. Also understood how the locks work.
