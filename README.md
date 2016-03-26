
# 1. How to build the kernel

* Build the revised kernel and push it into your device. Enter the linux/ directory and type


>>  ./build.sh tizen_tm1 USR

>>  sdb -d root on

>>  sdb push system_kernel.tar /opt/storage/sdcard/tizen-recovery.tar

>>  sdb shell reboot recovery

* Reboot your device with the kernel.

* Afterwards, compile the test code and also push it into your device. Enter the tests/ folder and type

>>  make

>>  sdb push test /home/developer/

* Test our new system call by typing

>>  sdb shell /home/developer/test


# 2. Description of design & implementation

* The new system call 'sys\_ptree' is defined in linux/kernel/ptree.c.

* The code consists of 3 functions:
  task\_to\_info; DFS; sys\_ptree.

 + 'task\_to\_info' transforms struct task\_struct into struct prinfo.

 + 'DFS' recursively traverses the task tree in preorder starting with init\_task. It calls task\_to\_info and saves struct prinfo into the buffer in preorder. In addition, the number of tasks are counted in this function.

 + 'sys\_ptree' checks error conditions and calls DFS, count\_task to fill in the buffer and return the number of running tasks.

* To sum up, when 'sys\_ptree' is called from the user space, it calls 'DFS' that traverses the task tree in preorder, transforms the tasks into struct prinfo one by one, and saves them into buffer. Then, it returns the number of tasks that was counted when 'DFS' is running.

# 3. Investigation of the process tree

## 3.1 Investigation of the process tree
	
* By executing the task for many times, we found that the systemd-udevd generates a child with the same name periodically. And an extra kworker is created and deleted from time to time.

## 3.2 Investigation of the process tree launching some applications

* When starting an application, camera application here as an example, 

  * New camera task appears as a child of launchpad-process. On the occassion of starting a camera application, a new task named camera appears.  

  * At the tail of the process tree, new kworkers and functional tasks appear newly. In the camera example, 2 kworkers, dcam\_flash_thread, img\_zoom\_thread, ipp\_cmd\_1 appeared newly.

## 3.3 Investigation of the launchpad and launchpad-loader

### 3.3a Experiments

 * Every time a new application is started, one or two related task is appeared as a child of launchpad and it remains when we force stopped the application(press the home button).

 * The tasks under the launchpad disappears when we do the clear all(long press the home button and press the clear all button). 

### 3.3b Explanation

* The launchpad saves the tasks of applications used in the order that they were started. 

* When the caller app calls the callee app, it sends the request to the launchpad. Then the launchpad checks if the caller app has the right, and launch the callee app if so. 

* If the launchpad decides to launch the callee app, the launchpad-loader launches the callee app with its process which was made beforehand.

* The launchpad remains a snapshot of the app even if it was force stopped so that the application can be launched from where it was stopped.


# 4. Lessons learned

* HURSUNGYUN (2014-19768) : I learned that how the system call and understood the linux kernel operates when it is called by user. 

* HURSUNGYUN (2014-

* HURSUNGYUN (2013-
