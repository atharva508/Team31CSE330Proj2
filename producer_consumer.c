#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/timekeeping.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/types.h>
static int buff_size=0;
static int p=0;
static int c=0;
static int uid=0;
int producer_ended = 0;
int consumer_ended = 0;
module_param(uid, int, 0);
module_param(buff_size, int, 0);
module_param(p, int, 0);
module_param(c, int, 0);
struct task_struct *buffer[100];
int64_t global_time = 0;
static int in = 0;// whre data should be inserted
static int out = 0; //from where the data should be removed
//mutex semaphore-makes sure that the buffer is only being modified by one thread
//full semaphore- keeps track of full indices
//empty semaphore- keeps track empty indices 
struct semaphore mutex, full,empty;
static struct task_struct *producer_thread;
static struct task_struct *consumer_thread[5];
int items_produced = 0;
int items_consumed = 0;

static int producer_func(void *data){
	int num;
        while(!kthread_should_stop()){
		struct task_struct *p;
		 for_each_process(p){
                        if(p->cred->uid.val==uid){
                        	num = down_interruptible(&empty);
				if(producer_ended){
				break;
				}
				num = down_interruptible(&mutex);
                        	items_produced++;
                        	buffer[in] = p;
                        	printk(KERN_INFO"[kProducer-1] Produce-Item#:%4d at buffer index: %3d for PID: %6d\n",items_produced,in,p->pid);
                        	in = (in+1)%buff_size;
				up(&mutex);
                        	up(&full);
                        }
                }
		break;
        }
        return 0;
}
int consumer_func(void *data){
	int num;
	//int threadNum = *(int*)data;

        while(!kthread_should_stop()){
		int64_t starting, currTime;
                struct task_struct *currTask;
		long time, hours, minutes, seconds;
		 num =down_interruptible(&full);//decreases full spots
		if(consumer_ended ==1){
		break;
		}
        	num =down_interruptible(&mutex);//decreases full spots
		items_consumed++;
                currTask = buffer[out];
                starting = currTask->start_time;
                currTime = ktime_get_ns();
		currTime = currTime-starting;
		time =(long) currTime;
                hours = time / 3600000000000;
                time = time % 3600000000000;
               	minutes = time / 60000000000;
                time = time % 60000000000;
                seconds = time / 1000000000;
                //not executed prperly
                printk(KERN_INFO"[kConsumer-1] Consumed Item#:%5d on buffer index:%3d :: PID:%10d  Elapsed Time %02lu:%02lu:%02lu\n",items_consumed,out,currTask->pid,hours,minutes,seconds);
		global_time+= currTime;
                //not executed prperly
                out = (out+1)%buff_size;
		up(&empty);
		up(&mutex);

        }
        return 0;

}

int producer_consumer_init(void){
        int j=0;
	printk(KERN_INFO"CSE330 Project 2 kernel module inserted\n");
	printk(KERN_INFO"module recieved the following inputs: UID:%d,Buffer-Size:%d No of Producer:%d No of Consumer:%d\n",uid,buff_size,p,c);
        sema_init(&mutex,1);
        sema_init(&empty,buff_size);
        sema_init(&full,0);
        //initializes the producers(0 or 1)
        if(p>0){
        producer_thread = kthread_run(producer_func, NULL, "Producer-1");
        }
        //initializes the consumers(0+)
        for(j=0;j<c;j++){
	//int threadNum = j+1;
        consumer_thread[j] = kthread_run(consumer_func, NULL, "Consumer Thread - %d",j+1);
	printk(KERN_INFO"[kConsumer-%d] kthread Consumer Created Successfully\n",j+1);

        }
        return 0;


}

//implement producer_func
void  producer_consumer_exit(void){
	int j;
        long time, hours, minutes, seconds;


         if(p>0){
                 producer_ended = 1;
                 up(&empty);
          }


        for(j=0;j<c;j++){
		consumer_ended=1;
		//kthread_stop(consumer_thread[j]);
		up(&full);
        }
  	 if(p>0){
		if(producer_ended!=1){
                kthread_stop(producer_thread);
		}
		printk(KERN_INFO"[kProducer-1] Producer Thread stopped\n");
	
        }
	for(j=0;j<c;j++){
		if(consumer_ended !=1){
                kthread_stop(consumer_thread[j]);
		}
		printk(KERN_INFO"[kConsumer-%d] Consumer Thread stopped\n",j+1);
		

        }
        printk(KERN_INFO"number of items produced = %d\n",items_produced);
        printk(KERN_INFO"number of items consumed = %d\n",items_consumed);
        //print total elapsed time(to be implemented);
	time =(long) global_time;
        hours = time / 3600000000000;
        time = time % 3600000000000;
        minutes = time / 60000000000;
        time = time % 60000000000;
        seconds = time / 1000000000;
	printk(KERN_INFO"The Total elapsed time of all processes for UID %d is   %02lu:%02lu:%02lu \n",uid,hours,minutes,seconds);
	printk(KERN_INFO"CSE Project 2 Kernel Module Removed\n");
}

module_init(producer_consumer_init);
module_exit(producer_consumer_exit);
MODULE_LICENSE("GPL");


