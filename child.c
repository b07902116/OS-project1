#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

void unit_time(void){ volatile unsigned long i; for(i=0;i<1000000UL;i++); }



int main(int argc, char *argv[]){
	char name[32];
	strcpy(name, argv[1]);
	int exec_time = atoi(argv[2]);
	int pid = getpid();
	printf("%s %d\n", name, pid);
	long long nsstart, nsend, NS = 1000000000;
	nsstart = syscall(334);
	for (int j = 0; j < exec_time; j++){
		unit_time();
	}
	nsend = syscall(334);
	syscall(335, pid, nsstart, nsend);
	exit(0);
}