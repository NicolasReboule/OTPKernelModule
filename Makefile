MODULE_NAME	=	otp_manager
MODULE_DIR	=	module.d

KERNEL_DIR	?=	/lib/modules/$(shell uname -r)/build
PWD		=	$(CURDIR)

TARGET_KO	=	$(MODULE_NAME).ko

obj-m	:=	$(MODULE_NAME).o
otp_manager-objs	:=	src/module.o src/crypto.o src/device.o

ccflags-y	+=	-I$(PWD)/include

# Compile sources into a kernel module
all:
	@make -C "$(KERNEL_DIR)" M="$(PWD)" modules
#	@mv -f *.o *.ko .*.cmd *.symvers *.order *.btf *.mod* $(MODULE_DIR)
#	@echo "$(pwd)/$(MODULE_DIR)/otp_manager.o" > $(MODULE_DIR)/otp_manager.order
#	@echo -e "$(pwd)/$(MODULE_DIR)/otp_manager.mod.c\n$(pwd)/$(MODULE_DIR)/module.o\n$(pwd)/$(MODULE_DIR)/crypto.o\n$(pwd)/$(MODULE_DIR)/device.o\n" > $(MODULE_DIR)/otp_manager.mod
#	@mv -f src/*.o src/.*.cmd src/.*.d $(MODULE_DIR)

# Clean compiled sources and the module
clean:
	@rm -rf $(MODULE_DIR)/* $(MODULE_DIR)/.*.cmd $(MODULE_DIR)/.*.d
	@rm -rf *.o .*.o.d *.ko .*.cmd *.symvers *.order *.btf *.mod*
	@rm -rf src/*.o src/.*.o.d src/.*.cmd

load:	all
	@sudo insmod $(TARGET_KO)

unload:	clean
	@sudo rmmod -f $(MODULE_NAME)

info:
	@ls -la /dev/ | grep otp
	@sudo modinfo $(TARGET_KO)

log:
	@sudo dmesg

logx:
	@sudo dmesg -Tw