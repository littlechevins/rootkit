#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/module.h>
#include <sys/types.h>


int main(void){
	setgid(0);
	setuid(0);
	execl("/bin/sh", "sh", 0);
}
