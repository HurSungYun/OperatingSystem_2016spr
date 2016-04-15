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
    + readers & writers waiting -> writer gets the lock (our team's own policy)

* **Currently no one grabbing the lock**
    + only readers waiting-> go get the lock
    + only writers waiting -> one of them gets the lock
    + readers & writers waiting -> writer gets the lock (our team's own policy)

 
 # 3. High level Design
 
 # 4. Implementation
 
 # 5. Lessons learned
 
 * HURSUNGYUN (2014-19768): I absolutely found that writing concurrent program with correctness is one of the most difficult tasks in the world.
 
 * YEONWOOKIM (2014-17184): 
 
 * EUNHYANGKIM (2013-13494): 
