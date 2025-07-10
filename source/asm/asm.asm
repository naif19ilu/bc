.section .bss
	.mem: .zero 30

.section .text

.globl _start

_start:
	leaq	.mem(%rip), %rax
	syscall
