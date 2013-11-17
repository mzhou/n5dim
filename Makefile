obj-m := n5dim.o
n5dim-objs := compat.o hook.o main.o my_lm3630_bl.o

all: modules

modules:
	make -C $(KERNEL_SOURCES) M=$(PWD) modules
clean:
	make -C $(KERNEL_SOURCES) M=$(PWD) clean
	rm -vf *.ko *.o

hook.o: hook.h mm_ksyms.h
main.o: hook.h my_lm3630_bl.h
my_lm3630_bl.o: lm3630_bl_ksyms.h my_lm3630_bl.h

.PHONY: modules clean
