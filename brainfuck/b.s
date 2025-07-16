.section .bss
	mem: .zero 30000

.section .text
.globl _start

# ++++++++[>+>++++<<-]>++>>+<[-[>>+<<-]+>>]>+[
#     -<<<[
#         ->[+[-]+>++>>>-<<]<[<]>>++++++[<<+++++>>-]+<<++.[-]<<
#     ]>.>+[>>]>+
# ]


_start:
	leaq	mem(%rip), %r8
	movb	$8, (%r8)
LB1:
	movzbl	(%r8), %eax
	cmpb	$0, %al
	je	LE1
	incq	%r8
	addb	$1, (%r8)
	incq	%r8
	addb	$4, (%r8)
	decq	$2, %r8
	subb	$1, (%r8)
	jmp	LB1
LE1:
	incq	%r8
	addb	$2, (%r8)
	addq	$2, %r8
	addb	$1, (%r8)
	decq	%r8
LB2:
	movzbl	(%r8), %eax
	cmpb	$0, %al
	je	LE2

