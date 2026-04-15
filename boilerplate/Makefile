obj-m += monitor.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules
	gcc engine.c -o engine -lpthread
	gcc supervisor.c -o supervisor
	gcc cpu_hog.c -o cpu_hog
	gcc memory_hog.c -o memory_hog
	gcc io_pulse.c -o io_pulse

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f engine supervisor cpu_hog memory_hog io_pulse
