.section .text

.globl _start

_start:
	jz	LE0
LE0:
	jmp	_start
