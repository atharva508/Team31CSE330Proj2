#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/timekeeping.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
static int buff_size=0;
static int p=0;
static int c=0;
static int uid=0;
module_param(buff_size, int, 0);
module_param(p, int, 0);
module_param(c, int, 0);
module_param(uid, int, 0);
struct task_struct buffer[50];
int in = 0;// whre data should be inserted
int out = 0; //from where the data should be removed
//mutex semaphore-makes sure that the buffer is only being modified by one thread
//full semaphore- keeps track of full indices
//empty semaphore- keeps track empty indices 
struct semaphore mutex, full,empty;
static struct task_struct *producer_thread;
static struct task_struct *consumer_thread[5];
int items_produced = 0;
int items_consumed = 0;


int producer_func(void *data){
	while(!kthread_should_stop()){
		if(down_interruptible(&mutex)){
		struct task_struct *p;
		for_each_process(p){
			if(down_interruptible(&empty)){
			items_produced++;
			buffer[in] = *p;
			printk("[kProducer-1] Produce-Item#:%d at buffer index: %d for PID: %d\n",items_produced,in,p->pid);
			in = (in+1)%buff_size;
			up(&full);
			}
		}
		up(&mutex);
		}		
	}
	return 0;
}

int consumer_func(void *data){
	while(!kthread_should_stop()){
		if(down_interruptible(&mutex)){//makes mutex 0 so no other thread alters
		if(down_interruptible(&full)){//decreases full spots
		struct task_struct *currTask;
		items_consumed++;
		currTask = &buffer[out];
		//struct timespec starting = currTask->start_time;
		//struct timespec currTime = ktime_get_ns();
		//not executed prperly
		printk("[kConsumer-<add num>] Consumed Item#:%5d on buffer index:%3d :: PID:%d  Elapsed Time <add time>",items_consumed,out,currTask->pid);
		out = (out+1)%buff_size;	
		up(&empty);
		}
		up(&mutex);
		}
	}
	return 0;

}
int producer_consumer_init(void){
        int i;
        printk("buff size = %d",buff_size);
        printk("number of producers= %d", p);
        printk("number of consumers =%d",c);
        printk("uid = %d",uid);
        sema_init(&mutex,1);
        sema_init(&empty,buff_size);
        sema_init(&full,0);
        //initializes the producers(0 or 1)
        for(i =0;i<p;i++){
        producer_thread = kthread_run(producer_func, NULL, "Producer-<add num>");
        }
        //initializes the consumers(0+)
        for(i =0;i<c;i++){
        consumer_thread[i] = kthread_run(consumer_func, NULL, "Consumer-<add num>");
        }
        return 0;


}

//implement producer_func
void  producer_consumer_exit(void){
	int i;
	for(i=0;i<p;i++){
		kthread_stop(producer_thread);
	}
	for(i =0;i<c;i++){
		kthread_stop(consumer_thread[i]);
	}

	//print total elapsed time(to be implemented)

}

module_init(producer_consumer_init);
module_exit(producer_consumer_exit);
MODULE_LICENSE("GPL");

