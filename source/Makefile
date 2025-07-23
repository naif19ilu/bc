# bc - brainfuck compiler
# Jul 13, 2025
# Makefile

objs  = main.o cxa.o fatal.o lexpa.o emu.o asm.o elf.o
flags = -Wall -Wextra -Wpedantic
opt   = -O0
std   = -std=c99
final = bc

all: $(final)

$(final): $(objs)
	cc	-o $(final) $(objs)
%.o: %.c
	cc	-c $< $(flags)
amd: a.s
	as	a.s
	ld	a.out -o asm
	./asm
arm: a.s
	aarch64-linux-gnu-as	a.s
	aarch64-linux-gnu-ld	a.out -o asm
	qemu-aarch64 ./asm
clear:
	rm	-rf $(objs) $(final) a.s a.out asm && clear
