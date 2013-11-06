obj-m += n5dim.o

all:
	make -C $(KERNEL_SOURCES) M=$(PWD) modules
clean:
	make -C $(KERNEL_SOURCES) M=$(PWD) clean
	rm -vf *.ko *.o

.PHONY: clean
