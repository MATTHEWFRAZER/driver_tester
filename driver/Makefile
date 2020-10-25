obj-m += driver_tester_module.o
driver_tester_module-y := detour_patching/driver_tester_detour_patching.o ../common/driver_tester_ioctl.o driver_tester.o
modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
