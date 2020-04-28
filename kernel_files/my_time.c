#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/ktime.h>

asmlinkage long long sys_my_time(void){
	struct timespec t;
	getnstimeofday(&t);
	return t.tv_sec * 1000000000 + t.tv_nsec;
}

//334
