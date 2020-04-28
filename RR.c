#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>


#define MAX_PROCESS_NUM 30
#define TIME_SLICE 500


typedef struct task{
	char name[32];
	int id, ready_time, exec_time, index;
}TASK;

TASK process[MAX_PROCESS_NUM];

char policy[8];
int process_num, now_num = 0, fin_num = 0, running = 0, run_time = 0, needwait = 0;
int pq[100000], qhead = 0, qtail = 0;


void unit_time(void){ volatile unsigned long i; for(i=0;i<1000000UL;i++); }

void read_input(void){
	scanf("%d", &process_num);
	for (int i = 0; i < process_num; i++){
		scanf("%s%d%d", process[i].name, &process[i].ready_time, &process[i].exec_time);
	}
	return;
}

int cmp(const void *a, const void *b){
	TASK *x = (TASK *) a;
	TASK *y = (TASK *) b;
	if (x->ready_time > y->ready_time)
		return 1;
	else
		return -1;
}

void q_push(int index){
	pq[qtail++] = index;
	return;
}

int q_size(void){
	return qtail - qhead;
}

int q_pop(void){
	if (q_size() == 0)
		return -1;
	else
		return pq[qhead++];
}

int q_peap(void){
	if (q_size() == 0)
		return -1;
	else
		return pq[qhead];
}


void priority(int pid, int next){
	struct sched_param SP;
	if (next == 0){
		SP.sched_priority = sched_get_priority_min(SCHED_FIFO);
	}
	else{
		SP.sched_priority = sched_get_priority_max(SCHED_FIFO);
	}
	sched_setscheduler(pid, SCHED_FIFO, &SP);
	return;
}


void run_next(void){
	int old, next;
	running = 0;
	run_time = 0;
	if (q_size() == 0)
		return;
	next = q_peap();
	priority(process[next].id, 1);
	running = 1;
	if (process[next].exec_time < TIME_SLICE)
		needwait = 1;
	return;
}



void signalhandler(int signum){
	wait(NULL);
	q_pop();
	fin_num++;
	needwait = 0;
	run_time = 0;
	running = 0;
	return;
}

void run_on_CPU(int pid, int index){
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(index, &mask);
	if (sched_setaffinity(pid, sizeof(mask), &mask) != 0){
		fprintf(stderr, "set fail\n");
	}
	return;
}

void new_process(void){
	int pid = fork();
	if (pid == 0){//child
		char tmp[32];
		sprintf(tmp, "%d", process[now_num].exec_time);
		execlp("./child", "child", process[now_num].name, tmp, (char *)NULL);
	}
	else{//parent
		process[now_num].id = pid;
		priority(pid, 0);
		run_on_CPU(pid, 1);
		return;
	}
}




int main(void){
	read_input();
	qsort(process, process_num, sizeof(TASK), cmp);
	for (int i = 0; i < process_num; i++){
		process[i].index = i;
	}
	run_on_CPU(getpid(), 0);
	signal(SIGCHLD, signalhandler);
	int time_count = 0, a, old;
	while (fin_num != process_num){
		while (now_num < process_num && time_count == process[now_num].ready_time){
			new_process();
			q_push(now_num++);
		}
		if (!running){
			run_next();
		}
		if (run_time == TIME_SLICE && !needwait){
			old = q_pop();
			priority(process[old].id, 0);
			process[old].exec_time -= TIME_SLICE;
			q_push(old);
			run_next();
		}
		unit_time();
		time_count++;
		run_time++;
	}
	return 0;
}