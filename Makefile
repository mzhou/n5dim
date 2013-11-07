obj-m := n5dim.o
n5dim-objs := compat.o hook.o main.o my_lm3630_bl.o

all: n5dim.ko.gz

n5dim.ko.gz: modules vermagic_strip
	./vermagic_strip <n5dim.ko | gzip -9 >n5dim.ko.gz

modules:
	make -C $(KERNEL_SOURCES) M=$(PWD) modules
clean:
	make -C $(KERNEL_SOURCES) M=$(PWD) clean
	rm -vf vermagic_strip *.ko *.o

hook.o: hook.h mm_ksyms.h
main.o: hook.h my_lm3630_bl.h
my_lm3630_bl.o: lm3630_bl_ksyms.h my_lm3630_bl.h

vermagic_strip: vermagic_strip.c
	$(CC) -Wall -Werror -pedantic -static -O3 \
			-o vermagic_strip vermagic_strip.c

.PHONY: modules clean
