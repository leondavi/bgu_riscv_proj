OBJ = main.o fftmisc.o  fourierf.o
FILE = main.c fftmisc.c  fourierf.c
CFLAGS = -static -O3 -g

CC = /opt/riscv/bin/riscv64-unknown-elf-c++ 
#CC = gcc

#small
DEFINES_1 =-DMAXSIZE_D=4096 -DMAXWAVES_D=4 -DINV=0 
NAME_1 = small_4_4096_0
DEFINES_2 =-DMAXSIZE_D=8192 -DMAXWAVES_D=4 -DINV=1 
NAME_2 = small_4_8192_1
#large
DEFINES_3 =-DMAXSIZE_D=32768 -DMAXWAVES_D=8 -DINV=0 
NAME_3 = small_8_32768_0
DEFINES_4 =-DMAXSIZE_D=32768 -DMAXWAVES_D=8 -DINV=1 
NAME_4 = small_8_32768_1

fft: ${OBJ} Makefile
	gcc  ${CFLAGS} ${OBJ} -o fft -lm
fftmisc.o: fftmisc.c
	gcc ${CFLAGS} -c fftmisc.c
fourierf.o: fourierf.c
	gcc ${CFLAGS} -c fourierf.c
main.o: main.c
	gcc ${CFLAGS} -c main.c -DDEBUG 

small:  
	${CC} fftmisc.c fourierf.c main.c ${DEFINES_1} -o fft_${NAME_1} -lm
	${CC} fftmisc.c fourierf.c main.c ${DEFINES_2} -o fft_${NAME_2} -lm
	cp fft_${NAME_1} ../../rv64gcc_obj/telcomm_fft_${NAME_1}
	cp fft_${NAME_2} ../../rv64gcc_obj/telcomm_fft_${NAME_2}
large:
	${CC} fftmisc.c fourierf.c main.c ${DEFINES_3} -o fft_${NAME_3} -lm
	${CC} fftmisc.c fourierf.c main.c ${DEFINES_4} -o fft_${NAME_4} -lm
	cp fft_${NAME_3} ../../rv64gcc_obj/telcomm_fft_${NAME_3}
	cp fft_${NAME_4} ../../rv64gcc_obj/telcomm_fft_${NAME_4}
clean:
	rm -rf *.o fft output*
