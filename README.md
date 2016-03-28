
# 1. How to build the kernel

* After cloning our git repository, check out to branch proj1

> git checkout proj1

* Build the revised kernel and push it into your device. Enter the linux/ directory and type

>  ./build.sh tizen_tm1 USR

>  sdb -d root on

>  sdb push system_kernel.tar /opt/storage/sdcard/tizen-recovery.tar

>  sdb shell reboot recovery

* Reboot your device with the kernel.

* Afterwards, compile the test code and also push it into your device. Enter the tests/ folder and type

>  make

>  sdb push test /home/developer/

* Test our new system call by typing

>  sdb shell /home/developer/test

* If you want to test with different buffer sizes, change the __MAX__ value in line 7 of tests/test.c

> vi tests/test.c

# 2. Description of design & implementation

* The new system call 'sys\_ptree' is defined in linux/kernel/ptree.c.

* The code consists of 3 functions:
  task\_to\_info; DFS; sys\_ptree.

 + 'task\_to\_info' transforms struct task\_struct into struct prinfo.

 + 'DFS' recursively traverses the task tree in preorder starting with init\_task. It calls task\_to\_info and saves struct prinfo into the buffer in preorder. In addition, the number of tasks are counted in this function.

 + 'sys\_ptree' checks error conditions and calls DFS to fill in the buffer and return the number of running tasks.

* To sum up, when 'sys\_ptree' is called from the user space, it calls 'DFS' that traverses the task tree in preorder, transforms the tasks into struct prinfo one by one, and saves them into buffer. Then, it returns the number of tasks that was counted when 'DFS' is running.

* nr includes swapper\0.

# 3. Investigation of the process tree

## 3.1 Investigations of the process tree
	
* By executing the program several times, we found that the task 'systemd-udevd' generates a child with the same name periodically. Also, kworker is created and deleted from time to time.

* We found that systemd-udevd manages the device and events. It communicates between the device and the kernel. 

* Therefore, its child is created when our test program sends a syscall message and the process is destroyed when we don't need it.

## 3.2 Investigations of the process tree launching some applications

* When starting an application, we used camera application as an example, 

  * New camera task appears as a child of launchpad-process. On the occassion of starting a camera application, a new task named camera appears. 

  * At the tail of the process tree, new kworkers and functional tasks appear. In the camera example, 2 kworkers, dcam\_flash\_thread, img\_zoom\_thread, ipp\_cmd\_1 appeared. 


## 3.3 Investigations of the launchpad and launchpad-loader

### 3.3a Experiments

 * Every time a new application is started, one or two related tasks appear as children of launchpad and they remain even if we stopped the application by pressing the home button.

 * The tasks under the launchpad disappear when we do the clear all(long press the home button and press the clear all button). 

### 3.3b Explanations

* The launchpad saves the tasks of applications used in the order that they were started. 

* The launchpad saves a snapshot of the app even if it was force stopped so that the application can be launched from where it was stopped.

* They use launchpad-loader to save the snapshot. In conclusion, it is used for pre-fork service to increase throughput and decrease latency.

* The launchpad also manage tasks. When the one app calls another app, it sends the request to the launchpad. 

* If the launchpad decides to launch another app, the launchpad-loader launches the app with its process which was made beforehand.


# 4. Lessons learned

* HURSUNGYUN (2014-19768): I learned that how the system call and understood the linux kernel operates when it is called by user. 

* YEONWOOKIM (2014-17184): By creating a new custom-made system call, we concretely understood the process of how system call operates in both user and kernel spaces.

* EUNHYANGKIM (2013-13494): Besides how system call works through the user and kernel space, we also learned about functions of some specific kernel tasks.
