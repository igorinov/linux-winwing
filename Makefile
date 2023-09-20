MDIR ?= /lib/modules/`uname -r`
KDIR ?= $(MDIR)/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

install:
	sudo cp *.ko $(MDIR)/kernel/drivers/hid
	sudo depmod -a
