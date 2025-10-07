obj-m := ddr.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc ddr_tool.c -o ddr_tool

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f ddr_tool
