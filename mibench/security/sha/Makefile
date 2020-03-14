# By default, the code is compiled for a "big endian" machine.
# To compile on a "little endian" machine set the LITTLE_ENDIAN flag.
# To make smaller object code, but run a little slower, don't use UNROLL_LOOPS.
# To use NIST's modified SHA of 7/11/94, define USE_MODIFIED_SHA

CC = /opt/riscv/bin/riscv32-unknown-elf-c++
#CC = gcc
#CC = /opt/riscv/bin/riscv64-unknown-elf-c++ -march=rv64g

CFLAGS = -O3 -Wall

sha:	sha_driver.o sha.o
	$(CC) ${CFLAGS} -o $@ sha_driver.o sha.o 
#	strip $@

clean:
	rm -rf *.o sha output*
