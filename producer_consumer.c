#include <linux/module.h>
#include <linux/kernel.h>
#include<linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include<linux/sched.h>
#include <linux/sched/signal.h>
module_param(buffSize, int, 0);
module_param(prod, int, 0);
module_param(cons, int, 0);
module_param(uid, int, 0);
struct task_struct buffer[buffSize];
int in = 0;
int out = 0; 
struct semaphore mutex, full,empty;
static struct task_struct *producer_thread;
static struct task_struct *consumer_thread[cons];

int producer_consumer_init(void){
	sema_init(&mutex,1);
	sema_init(&empty,buffSize);
	sema_init(&full,0);
	int i;
	for(i =0;i<prod;i++){
	producer_thread = kthread_run(producer_func, NULL, "Producer-<add num>");
	}
	for(i =0;i<cons;i++){
        consumer_thread[i] = kthread_run(consumer_func, NULL, "Consumer-<add num>");
        }
	return 0;


}

void consumer_func(){
	while(true){
	down_interruptible(&mutex);
	down_interruptible(&full);
	task_struct currTask = buffer[out];
	struct timespec starting = currTask->start_time;
	struct timespec currTime = ktime_get_ns();
	print(currtime);
	up(&full);
	up(&mutex);
	}
	

}
void producer_consumer_exit(void){
	int i,j;
	for(int j=0;j<prod;j++){
		kthread_stop(producer_thread);
	}

	for(i =0;i<cons;i++){
		kthread_stop(consumer_thread[i]);
	}

	//print total elapsed time
	sema_destroy(&mutex);
	sema_destroy(&full);
	sema_destroy(&empty);

}

module_init(producer_consumer_init);

module_exit(producer_consumer_exit);

