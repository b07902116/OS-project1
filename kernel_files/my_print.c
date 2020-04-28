#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage void sys_my_print(int pid, long long start, long long end){
	long long NS = 1000000000;
	printk("[Project1] %d %lld.%09lld %lld.%09lld\n", pid, start / NS, start % NS, end / NS, end % NS);
}

//335
