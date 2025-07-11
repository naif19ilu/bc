.section .text

.globl _start

_start:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$3, %rsp

