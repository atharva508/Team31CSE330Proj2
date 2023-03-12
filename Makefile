obj-m := producer_consumer.o

all:
	make -C process_gen/ all
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C process_gen/ clean
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
