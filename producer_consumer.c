#include <linux/module.h>
#include <linux/init.h>
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
int producer_ended = 0;
int consumer_ended = 0;
module_param(uid, int, 0);
module_param(buff_size, int, 0);
module_param(p, int, 0);
module_param(c, int, 0);
struct task_struct buffer[50];
static int in = 0;// whre data should be inserted
static int out = 0; //from where the data should be removed
//mutex semaphore-makes sure that the buffer is only being modified by one thread
//full semaphore- keeps track of full indices
//empty semaphore- keeps track empty indices 
struct semaphore mutex, full,empty;
static struct task_struct *producer_thread;
static struct task_struct consumer_thread[5];
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
                        	buffer[in] = *p;
                        	printk("[kProducer-1] Produce-Item#:%d at buffer index: %d for PID: %d\n",items_produced,in,p->pid);
                        	in = (in+1)%buff_size;
				up(&mutex);
                        	up(&full);
                        }
                }
		break;
        }
	printk("Producer ended my bitch\n");
        return 0;
}
int consumer_func(void *data){
	int num;
	int counter=0;
        while(!kthread_should_stop()){
                struct task_struct *currTask;
		num =down_interruptible(&full);//decreases full spots
		printk("entering counter - %d\n",++counter);
		if(consumer_ended==1){
			printk("consumer ended bitch\n");
			break;
		}
                num =down_interruptible(&mutex);//makes mutex 0 so no other thread alters
                items_consumed++;
                currTask = &buffer[out];
                //struct timespec starting = currTask->start_time;
                //struct timespec currTime = ktime_get_ns();
                //not executed prperly
                printk("[kConsumer-1] Consumed Item#:%5d on buffer index:%3d :: PID:%d  Elapsed Time <add time>",items_consumed,out,currTask->pid);
                out = (out+1)%buff_size;
		up(&mutex);
                up(&empty);

        }
	printk("consumer actually ended LOL\n");
        return 0;

}

int producer_consumer_init(void){
        int j;
        printk("buff size = %d\n",buff_size);
        printk("number of producers= %d\n", p);
        printk("number of consumers =%d\n",c);
        printk("uid = %d\n",uid);
        sema_init(&mutex,1);
        sema_init(&empty,buff_size);
        sema_init(&full,0);
        //initializes the producers(0 or 1)
        if(p>0){
        producer_thread = kthread_run(producer_func, NULL, "Producer-1");
        }
        //initializes the consumers(0+)
        for(j =0;j<c;j++){
        consumer_thread[j] = *kthread_run(consumer_func, NULL, "Consumer Thread - %d",j+1);
	printk("consumer thread-%d started\n",j+1);

        }
        return 0;


}

//implement producer_func
void  producer_consumer_exit(void){
      int i;
      int j;
	printk("number of items produced = %d\n",items_produced);
    	
        for(i = 0; i<c;i++){
		consumer_ended=1;
                printk("about to end life\n");
                up(&full);
        }

	 if(p>0){
                 producer_ended = 1;
                 up(&empty);
          }



	if(p>0){
        	kthread_stop(producer_thread);
        }

        for(j =0;j<c;j++){
                kthread_stop(&consumer_thread[j]);
        }

	

        //print total elapsed time(to be implemented)
}

module_init(producer_consumer_init);
module_exit(producer_consumer_exit);
MODULE_LICENSE("GPL");


