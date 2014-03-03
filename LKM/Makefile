# Nick Kalamvokis and Etienne Scraire CS3013-C14

obj-m := mailboxSetup_LKM.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
