#LINUX_SRC 应该指定开发板所运行的Linux内核源码的路径
LINUX_SRC = ${shell pwd}/./kernel/

#交叉编译器
CROSS_COMPILE=${shell pwd}/./prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc-8.3.0
#INST_PATH=/tftp
 
PWD := $(shell pwd)
 
EXTRA_CFLAGS+=-DMODULE
 
obj-m += hello.o
 
modules:
	@echo ${LINUX_SRC}
	@make -C $(LINUX_SRC) M=$(PWD) modules
	@make clear
 
uninstall:
	rm -f ${INST_PATH}/*.ko
 
install: uninstall
	cp -af *.ko ${INST_PATH}
 
clear:
	@rm -f *.o *.cmd *.mod.c
	@rm -rf  *~ core .depend  .tmp_versions Module.symvers modules.order -f
	@rm -f .*ko.cmd .*.o.cmd .*.o.d
 
clean: clear
	@rm -f  *.ko