#CC = /opt/riscv/bin/riscv64-unknown-elf-c++
CC = gcc
FLAGS = -D DEBUG

patricia: patricia.c patricia_test.c
	${CC} -static patricia.c patricia_test.c -O3 -o patricia

clean:
	rm -rf patricia output*
