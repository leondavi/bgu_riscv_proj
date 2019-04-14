FILE1 = qsort_small.c
FILE2 = qsort_large.c

CC = /opt/riscv/bin/riscv64-unknown-elf-c++
#cc = gcc

all: qsort_small qsort_large

qsort_small: qsort_small.c Makefile
	${CC} -static ${FILE1} -O3 -o qsort_small -lm
qsort_large: qsort_large.c Makefile
	${CC} -static ${FILE2} -O3 -o qsort_large -lm

clean:
	rm -rf qsort_small qsort_large output*
