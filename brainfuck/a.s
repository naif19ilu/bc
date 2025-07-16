.section .bss
	memory: .zero 30000
.section .text
.globl _start
_start:
	leaq	memory(%rip), %rax
	movq	%rax, %r8
	addb	$8, (%r8)
LB1:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE1
	addq	$1, %r8
	addb	$1, (%r8)
	addq	$1, %r8
	addb	$4, (%r8)
	subq	$2, %r8
	subb	$1, (%r8)
	jmp	LB1
LE1:
	addq	$1, %r8
	addb	$2, (%r8)
	addq	$2, %r8
	addb	$1, (%r8)
	subq	$1, %r8
LB14:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE14
	subb	$1, (%r8)
LB16:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE16
	addq	$2, %r8
	addb	$1, (%r8)
	subq	$2, %r8
	subb	$1, (%r8)
	jmp	LB16
LE16:
	addb	$1, (%r8)
	addq	$3, %r8
	jmp	LB14
LE14:
	addb	$1, (%r8)
LB26:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE26
	subb	$1, (%r8)
	subq	$3, %r8
LB29:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE29
	subb	$1, (%r8)
	addq	$1, %r8
LB32:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE32
	addb	$1, (%r8)
LB34:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE34
	subb	$1, (%r8)
	jmp	LB34
LE34:
	addb	$1, (%r8)
	addq	$1, %r8
	addb	$2, (%r8)
	addq	$3, %r8
	subb	$1, (%r8)
	subq	$4, %r8
	jmp	LB32
LE32:
LB44:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE44
	jmp	LB44
LE44:
	addq	$2, %r8
	addb	$6, (%r8)
LB48:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE48
	subq	$2, %r8
	addb	$5, (%r8)
	addq	$2, %r8
	subb	$1, (%r8)
	jmp	LB48
LE48:
	addb	$1, (%r8)
	subq	$2, %r8
	addb	$2, (%r8)
	movq	$1, %rax
	movq	$1, %rdi
	movq	$1, %rdx
	movq	%r8, %rsi
	syscall
LB58:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE58
	subb	$1, (%r8)
	jmp	LB58
LE58:
	subq	$2, %r8
	jmp	LB29
LE29:
	addq	$1, %r8
	movq	$1, %rax
	movq	$1, %rdi
	movq	$1, %rdx
	movq	%r8, %rsi
	syscall
	addq	$1, %r8
	addb	$1, (%r8)
LB67:
	movzbl	(%r8), %eax
	cmpl	$0, %eax
	je	LE67
	addq	$3, %r8
	jmp	LB67
LE67:
	addb	$1, (%r8)
	jmp	LB26
LE26:
	movq	$60, %rax
	movq	$0, %rdi
	syscall
