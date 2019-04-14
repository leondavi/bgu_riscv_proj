FILE = bitcnt_1.c bitcnt_2.c bitcnt_3.c bitcnt_4.c bitcnts.c bitfiles.c bitstrng.c bstr_i.c 
#CC = /opt/riscv/bin/riscv64-unknown-elf-c++
CC = gcc
FLAGS = -D DEBUG

bitcnts: ${FILE} Makefile
	${CC} -static ${FILE} -O3 -o bitcnts 

clean:
	rm -rf bitcnts output*
