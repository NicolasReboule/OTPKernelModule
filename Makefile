MODULE_NAME	=	otp_manager
MODULE_DIR	=	module.d

KERNEL_DIR	?=	/lib/modules/$(shell uname -r)/build
PWD		=	$(CURDIR)

TARGET_KO	=	$(MODULE_DIR)/$(MODULE_NAME).ko

obj-m	:=	$(MODULE_NAME).o
otp_manager-objs	:=	module.o crypto.o

# Compile sources into a kernel module
all:	clean
	@make -C "$(KERNEL_DIR)" M="$(PWD)" modules
	@mv -f *.o *.ko .*.cmd *.symvers *.order *.btf *.mod* $(MODULE_DIR)

# Clean compiled sources and the module
clean:
	@rm -rf $(MODULE_DIR)/* $(MODULE_DIR)/.*.cmd


load:	all
	@sudo insmod $(TARGET_KO)

unload:	clean
	@sudo rmmod $(MODULE_NAME)

info:
	@sudo modinfo $(TARGET_KO)

log:
	@sudo dmesg

logx:
	@sudo dmesg -Tw