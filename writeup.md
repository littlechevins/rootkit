Everything required in the midpoint writeup

Group 4 Rootkit Midpoint Submission
===================================
1. Installation
---------------
We install the rootkit by loading the “rootkit.ko” kernel module:

    `#!/bin/sh
    REMOTE_IP=$1
    set -e
    cc -o ./src/suid ./src/suid.c
    chmod 4111 ./src/suid
    kldload -v ./src/kld/rootkit.ko
    echo "rootkit installed"
    exit 0`

2. Privilege Escalation
-----------------------
Privilege escalation is achieved by executing the binary “./src/setup”.
This will call fork(), then the child process will call system(“./src/suid”), spawning a root shell using the “suid” binary.

3. Concealment
--------------
We attempt to conceal our rootkit using the parent process of the fork mentioned above, which uses the KLD “rootkit.ko”. The KLD we loaded before allows us to remove running processes from the allproc list if we know the name. To hide our rootkit from programs such as ps, we need to remove all the processes used by our rootkit (“sh”, “setup” and “suid”).

The parent process will wait one second to give the child process time to execute.
The parent makes a system call to remove”sh” processes (i.e the spawned shell)
The parent makes a system call to remove ”setup” processes (i.e itself and the child process)
The parent makes a system call to remove the ”suid” process (i.e the binary called by the child of this process)

In addition to this, all scripts remove any used files at the end of their execution

4. Bonus Marks
--------------
# 4a) Remote shell spawn #
To spawn a remote shell we will use a hook of icmp_input along with modifications to execve. The remote program will create a simple ssh connection and provide a signal to the rootkit which will then spawn a root shell and connect back to the remote client using netcat.
# 4b) Keylogging #
To achieve keylogging we will hook the read, pread, readv and preadv system calls to write read data. We plan to test direct file writes and character devices as methods of storing the information in a subtle way. Then, using part or all of our remote shell functionality we will transfer the saved data/file to the remote machine when requested.
# 4c) Boot Persistence #
To achieve boot persistence we can add the rootkit kernel module to /boot/modules and add a line to /boot/loader.conf to load the module at boot-time. This would be done upon rootkit installation. We plan to have a plan on hiding part of the written /boot/loader.conf file to preserve boot modules and also hide a rather obvious sign of a rootkit.


Group 4 Rootkit Final Submission
===================================

What changes were made since the midpoint deadline

Since the midterm our installation is very similar, we compile the elevation script, provide permissions and then load a process_hiding kld module which will also hide itself. When the elevation script is run it will attempt to conceal itself using the previously loaded kld module to hide itself.


Design decisions for rootkit detection


What methods are being detected and how


Design decisions & justifications for rootkit & rootkit detector decisions

Rootkit design justifications

  Process Hiding:
  1. We lock the allproc list and remove any references to the input string of the process we want to hide. We keep looping even after finding and removing the required process since any child processes spawned also have a copy of the allproc list.
  2. Since processes can be located via their hash number in the pidhashtbl, we call pfind as well to remove the entry.
  3. Since the nprocs variable stores the number of running processes, we must decrement this also to remove traces of extra running processes.
  4. We now need to hide KLD's from the system. When a KLD is loaded into the kernel, we are loading a linker file that contains one or more kernel modules. This results in the the system storing the KLD on two different lists: 'linker_files' and 'modules'.
  5. Same as removing processes from the allproc list, we iterate through the module and linker_file lists looking for "name.ko", if we find it, we decrement the nextid and remove it from the list.
  6. During testing we noticed that we have to perform '(&linker_files)->tqh_first->refs--;' twice to properly decrement the references to kernel. This may be a result of the kld's children.

  After sucessfully hiding the kld, we were unable to unload the kld as it could not be found. The only way around this was to reboot.

  Not done:
  1. Parent process's child list
  2. Parent process's process-group list


  Privilege escalation

  1. During installation of the rootkit, we compile the elevate code and ensure that execution permissions are enabled via chod 4111.
  2. Elevate will find the syscall num of our previously installed process_hiding kld and then call a syscall along with its own pid to hide itself.
  3. It will then elevate its uid and guid to 0, this is the simplest way to elevate its uid
  4. It then calls execl with bin/sh to spawn a shell

Rootkit detector design justifications
  Read hook detection:
  1. Detection is run with ./detect
  2. Code checks that the function sys_read actually points to its correct call number by referencing the kernel-s in-memory symbol table
  3. Alerts if the location isn't what it should be
  4. Doesn't check for the rootkit referencing the symbol table on the filesystem

  Port detection:
  1. Basic script that checks which ports are open
  2. Breaks and gives an alert if any port within port_hiding_args is open 
