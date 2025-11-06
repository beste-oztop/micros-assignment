all:
	as --32 fifos.s -o fifos.o
	as --32 context_switch.s -o context_switch.o
	gcc -m32 -g -O0 -std=gnu99 -fno-stack-protector -fno-builtin -nostdinc -c kentry.c -o kentry.o
	ld -m elf_i386 -T fifos.ld fifos.o context_switch.o kentry.o -o fifos.elf
clean:
	rm kentry.o fifos.elf fifos.o context_switch.o

