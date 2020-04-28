#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>


#define MAX_PROCESS_NUM 30


typedef struct task{
	char name[32];
	int id, ready_time, exec_time, index;
}TASK;

TASK process[MAX_PROCESS_NUM];

char policy[8];
int process_num, now_num = 0, fin_num = 0, running = 0, run_index, syn_time = 0, idle = 1;
int minheap[MAX_PROCESS_NUM + 1], heapnum = 0;


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
	else if (x->ready_time == y-> ready_time && x->exec_time > y->exec_time)
		return 1;
	else
		return -1;
}


void heap_swap(int i, int j){
	int tmp = minheap[i];
	minheap[i] = minheap[j];
	minheap[j] = tmp;
	return;
}

int heap_insert(int index){
	if (heapnum == 0){
		minheap[0] = index;
		heapnum++;
		return -1;
	}
	int pos = heapnum, parent, child, tmp;
	int old = minheap[0];
	//process[minheap[0]].exec_time -= run_time;
	minheap[heapnum++] = index;
	while (pos != 0){
		parent = minheap[(pos - 1) / 2];
		child = minheap[pos];
		if ((process[parent].exec_time > process[child].exec_time) ||
			(process[parent].exec_time == process[child].exec_time && process[parent].ready_time > process[child].ready_time)){
			heap_swap(pos, (pos - 1) / 2);
			pos = (pos - 1) / 2;
		}
		else
			break;
	}
	return old;
}

int heap_del_root(void){
	if (heapnum == 0)
		return -1;
	int pos = 0, parent, left, right, tmp;
	int old = minheap[0];
	minheap[pos] = minheap[--heapnum];
	while (pos < heapnum){
		parent = minheap[pos];
		left = pos * 2 + 1 >= heapnum? -1 : minheap[pos * 2 + 1];
		right = pos * 2 + 2 >= heapnum? -1 : minheap[pos * 2 + 2];
		if (left == -1)
			break;
		else if (right == -1){
			if (process[left].exec_time < process[parent].exec_time){
				heap_swap(pos, pos * 2 + 1);
				pos = pos * 2 + 1;
			}
			else if ((process[left].exec_time == process[parent].exec_time) && (process[left].ready_time < process[parent].ready_time)){
				heap_swap(pos, pos * 2 + 1);
				pos = pos * 2 + 1;
			}
			else
				break;
		}
		else{
			if ((process[left].exec_time < process[right].exec_time) ||
				(process[left].exec_time == process[right].exec_time && process[left].ready_time < process[right].ready_time)){
				if ((process[left].exec_time < process[parent].exec_time) ||
					(process[left].exec_time == process[parent].exec_time) && (process[left].ready_time < process[parent].ready_time)){
					heap_swap(pos, pos * 2 + 1);
					pos = pos * 2 + 1;
				}
				else
					break;
			}
			else{
				if ((process[right].exec_time < process[parent].exec_time) ||
					(process[right].exec_time == process[parent].exec_time) && (process[right].ready_time < process[parent].ready_time)){
					heap_swap(pos, pos * 2 + 2);
					pos = pos * 2 + 2;
				}
				else
					break;
			}
		}
	}
	return old;
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



void change_to_fastest(int old){
	if (heapnum == 0){
		idle = 1;
		return;
	}
	if (old != -1)
		priority(process[old].id, 0);
	run_index = heap_del_root();
	priority(process[run_index].id, 1);
	if (idle){//from no one running wake up
		syn_time = process[run_index].ready_time;
		idle = 0;
	}
	running = 1;
	return;
}



void signalhandler(int signum){
	wait(NULL);
	running = 0;
	fin_num++;
	syn_time += process[run_index].exec_time;
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
	int time_count = 0;
	while (fin_num != process_num){
		//maintain time consistency of two CPUs
		if (time_count < syn_time)
			time_count = syn_time;
		while (now_num < process_num && time_count >= process[now_num].ready_time){
			new_process();
			heap_insert(now_num++);
		}
		if (!running)	change_to_fastest(-1);
		
		//when a process die, check
		unit_time();
		time_count++;
	}
	return 0;
}



