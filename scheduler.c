#include <stdio.h>
#include <sched.h>
#include <unistd.h>

int main(void){
	char policy[8];
	scanf("%s", policy);
	fflush(stdin);
	struct sched_param SP;
	SP.sched_priority = sched_get_priority_max(SCHED_FIFO);
	sched_setscheduler(getpid(), SCHED_FIFO, &SP);
	switch (policy[0]){
	case 'F':
		execlp("./fifo", "fifo", (char *) 0);
		break;
	case 'R':
		execlp("./rr", "rr", (char *) 0);
		break;
	case 'S':
		execlp("./sjf", "sjf", (char *) 0);
		break;
	case 'P':
		execlp("./psjf", "psjf", (char *) 0);
		break;
	default :
		fprintf(stderr, "no such scheduling case\n");
		break;
	}
	return 0;
}