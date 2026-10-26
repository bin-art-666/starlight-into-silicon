KERNELDIR ?= /tmp/lab4-kernel
ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-
obj-m := beep.o

.PHONY: all module app clean
all: module app
module:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) LOCALVERSION=-g3dc0a4b HOSTCFLAGS="-O2 -fcommon" modules
app:
	$(CROSS_COMPILE)gcc -static -O2 -Wall -Wextra -Werror beepApp.c -o beepApp
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -f beepApp
