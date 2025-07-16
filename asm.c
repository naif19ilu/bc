/* bc - brainfuck compiler
 * Jul 15, 2025 Happy birthday!!
 * Assembly generator (-S)
 */
#include "asm.h"
#include "fatal.h"

struct asmgen
{
	FILE      *file;
	struct    { char *reg;  char prefix; unsigned char factor; } amd;
	struct    { char *load; char *store; char prefix; } arm;
	enum arch arch;
};

static const char *const Headers[] =
{
	".section .bss\n"
	"\tmemory: .zero %ld\n"
	".section .text\n"
	".globl _start\n"
	"_start:\n"
	"\tleaq\tmemory(%rip), %rax\n"
	"\tmovq\t%rax, %r8\n",

	".section .bss\n"
	"\tmemory: .skip %ld\n"
	".section .text\n"
	".globl _start\n"
	"_start:\n"
	"\tadrp\tx9, memory\n"
	"\tadd\tx9,x9,:lo12:memory\n"
};

static const char *const Footers[] =
{
	"\tmovq\t$60, %rax\n"
	"\tmovq\t$0, %rdi\n"
	"\tsyscall\n",

	"\tmov\tx8, #9\n"
	"\tmov\tx0, $0\n"
	"\tsvc\t#0\n"
};

static void get_arch_family (struct asmgen *asmg, const unsigned char cellSize)
{
	if (asmg->arch == ARCH_AMD64)
	{
		switch (cellSize)
		{
			case 1: { asmg->amd.reg = "al" ; asmg->amd.prefix = 'b'; break; }
			case 2: { asmg->amd.reg = "ax" ; asmg->amd.prefix = 'w'; break; }
			case 4: { asmg->amd.reg = "eax"; asmg->amd.prefix = 'l'; break; }
			case 8: { asmg->amd.reg = "rax"; asmg->amd.prefix = 'q'; break; }
		}
		asmg->amd.factor = cellSize;
		return;
	}

	switch (cellSize)
	{
		case 1: { asmg->arm.load = "ldrb"; asmg->arm.store = "strb"; asmg->arm.prefix = 'w'; break; }
		case 2: { asmg->arm.load = "ldrh"; asmg->arm.store = "strh"; asmg->arm.prefix = 'w'; break; }
		case 4: { asmg->arm.load = "ldr";  asmg->arm.store = "str";  asmg->arm.prefix = 'w'; break; }
		case 8: { asmg->arm.load = "ldr";  asmg->arm.store = "str";  asmg->arm.prefix = 'x'; break; }
	}
}

inline static void amd64_emmit_inc (const struct asmgen *asmg, const unsigned long group) { fprintf(asmg->file, "\tadd%c\t$%ld, (%%r8)\n", asmg->amd.prefix, group); }
inline static void amd64_emmit_dec (const struct asmgen *asmg, const unsigned long group) { fprintf(asmg->file, "\tsub%c\t$%ld, (%%r8)\n", asmg->amd.prefix, group); }
inline static void amd64_emmit_nxt (const struct asmgen *asmg, const unsigned long group) { fprintf(asmg->file, "\taddq\t$%ld, %%r8\n", group * asmg->amd.factor); }
inline static void amd64_emmit_prv (const struct asmgen *asmg, const unsigned long group) { fprintf(asmg->file, "\tsubq\t$%ld, %%r8\n", group * asmg->amd.factor); }
inline static void amd64_emmit_out (const struct asmgen *asmg, const unsigned long group)
{
	static const char *const template =
		"\tmovq\t$1, %rax\n"
		"\tmovq\t$1, %rdi\n"
		"\tmovq\t$1, %rdx\n"
		"\tmovq\t%r8, %rsi\n"
		"\tsyscall\n";
	for (unsigned long i = 0; i < group; i++)
	{
		fprintf(asmg->file, "%s", template);
	}
}
inline static void amd64_emmit_inp (const struct asmgen *asmg, const unsigned long group)
{
	static const char *const template =
		"\tmovq\t$0, %rax\n"
		"\tmovq\t$0, %rdi\n"
		"\tmovq\t$1, %rdx\n"
		"\tmovq\t%r8, %rsi\n"
		"\tsyscall\n";
	for (unsigned long i = 0; i < group; i++)
	{
		fprintf(asmg->file, "%s", template);
	}
}
inline static void amd64_emmit_lbr (const struct asmgen *asmg, const unsigned long branch)
{
	if (asmg->amd.prefix == 'q')
	{
		static const char *const template =
			"LB%ld:\n"
			"\tmovq\t(%r8), %%rax\n"
			"\tcmpq\t$0, %%rax\n"
			"\tje\tLE%ld\n";
		fprintf(asmg->file, template, branch, branch);
		return;
	}

	static const char *const template =
		"LB%ld:\n"
		"\tmovzbl\t(%r8), %%eax\n"
		"\tcmp%c\t$0, %%%s\n"
		"\tje\tLE%ld\n";
	fprintf(asmg->file, template, branch, asmg->amd.prefix, asmg->amd.reg, branch);
}
inline static void amd64_emmit_rbr (const struct asmgen *asmg, const unsigned long branch)
{
	static const char *const template =
		"\tjmp\tLB%ld\n"
		"LE%ld:\n";
	fprintf(asmg->file, template, branch, branch);
}

void asm_gen_asm (const struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned char cellSize, const enum arch arch)
{
	struct asmgen asmg = { .file = fopen(filename, "w"), .arch = arch };
	if (!asmg.file) { fatal_file_ops(filename); }

	get_arch_family(&asmg, cellSize);
	fprintf(asmg.file, Headers[arch], (unsigned long) (tapeSize * cellSize));

	for (size_t i = 0; i < stream->length; i++)
	{
		const struct token *token = &stream->stream[i];
		switch (token->meta.mnemonic)
		{
			case '+': emmit_inc(&asmg, token->groupSize);      break;
			case '-': emmit_dec(&asmg, token->groupSize);      break;
			case '>': emmit_nxt(&asmg, token->groupSize);      break;
			case '<': emmit_prv(&asmg, token->groupSize);      break;
			case '.': emmit_out(&asmg, token->groupSize);      break;
			case ',': emmit_inp(&asmg, token->groupSize);      break;
			case '[': emmit_lbr(&asmg, token->parnerPosition); break;
			case ']': emmit_rbr(&asmg, token->parnerPosition); break;
		}
	}

	fprintf(asmg.file, "%s", Footers[arch]);
	fclose(asmg.file);
}
