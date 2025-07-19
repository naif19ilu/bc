/* bc - brainfuck compiler
 * Jul 15, 2025 Happy birthday!!
 * Assembly generator (-S)
 */
#include "asm.h"
#include "fatal.h"

struct asmgen
{
	FILE          *file;
	struct        { char *reg;  char prefix; } amd;
	struct        { char *load; char *store; char prefix; } arm;
	unsigned char cellwidth;
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

	"\tmov\tx8, #93\n"
	"\tmov\tx0, #0\n"
	"\tsvc\t#0\n"
};

static void get_arch_family (struct asmgen *asmg, const unsigned char cellSize, const enum arch arch)
{
	asmg->cellwidth = cellSize;

	if (arch == ARCH_AMD64)
	{
		switch (cellSize)
		{
			case 1: { asmg->amd.reg = "al" ; asmg->amd.prefix = 'b'; break; }
			case 2: { asmg->amd.reg = "ax" ; asmg->amd.prefix = 'w'; break; }
			case 4: { asmg->amd.reg = "eax"; asmg->amd.prefix = 'l'; break; }
			case 8: { asmg->amd.reg = "rax"; asmg->amd.prefix = 'q'; break; }
		}
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

static void amd64_emmit_inc (const struct asmgen*, const unsigned long);
static void amd64_emmit_dec (const struct asmgen*, const unsigned long);

static void amd64_emmit_nxt (const struct asmgen*, const unsigned long);
static void amd64_emmit_prv (const struct asmgen*, const unsigned long);

static void amd64_emmit_out (const struct asmgen*, const unsigned long);
static void amd64_emmit_inp (const struct asmgen*, const unsigned long);

static void amd64_emmit_lbr (const struct asmgen*, const unsigned long);
static void amd64_emmit_rbr (const struct asmgen*, const unsigned long);

static void arm64_emmit_inc (const struct asmgen*, const unsigned long);
static void arm64_emmit_dec (const struct asmgen*, const unsigned long);

static void arm64_emmit_nxt (const struct asmgen*, const unsigned long);
static void arm64_emmit_prv (const struct asmgen*, const unsigned long);

static void arm64_emmit_out (const struct asmgen*, const unsigned long);
static void arm64_emmit_inp (const struct asmgen*, const unsigned long);

static void arm64_emmit_lbr (const struct asmgen*, const unsigned long);
static void arm64_emmit_rbr (const struct asmgen*, const unsigned long);

void asm_gen_asm (const struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned char cellSize, const enum arch arch)
{
	struct asmgen asmg = { .file = fopen(filename, "w") };
	if (!asmg.file) { fatal_file_ops(filename); }

	get_arch_family(&asmg, cellSize, arch);
	fprintf(asmg.file, Headers[arch], (unsigned long) (tapeSize * cellSize));

	typedef void (*emmiter_t) (const struct asmgen*, const unsigned long);

	const emmiter_t emmiters[] =
	{
		(arch == ARCH_AMD64) ? amd64_emmit_inc : arm64_emmit_inc,
		(arch == ARCH_AMD64) ? amd64_emmit_dec : arm64_emmit_dec,
		(arch == ARCH_AMD64) ? amd64_emmit_nxt : arm64_emmit_nxt,
		(arch == ARCH_AMD64) ? amd64_emmit_prv : arm64_emmit_prv,
		(arch == ARCH_AMD64) ? amd64_emmit_out : arm64_emmit_out,
		(arch == ARCH_AMD64) ? amd64_emmit_inp : arm64_emmit_inp,
		(arch == ARCH_AMD64) ? amd64_emmit_lbr : arm64_emmit_lbr,
		(arch == ARCH_AMD64) ? amd64_emmit_rbr : arm64_emmit_rbr,
	};

	for (size_t i = 0; i < stream->length; i++)
	{
		const struct token *token = &stream->stream[i];
		switch (token->meta.mnemonic)
		{
			case '+': emmiters[0] (&asmg, token->groupSize);      break;
			case '-': emmiters[1] (&asmg, token->groupSize);      break;
			case '>': emmiters[2] (&asmg, token->groupSize);      break;
			case '<': emmiters[3] (&asmg, token->groupSize);      break;
			case '.': emmiters[4] (&asmg, token->groupSize);      break;
			case ',': emmiters[5] (&asmg, token->groupSize);      break;
			case '[': emmiters[6] (&asmg, token->parnerPosition); break;
			case ']': emmiters[7] (&asmg, token->parnerPosition); break;
		}
	}

	fprintf(asmg.file, "%s", Footers[arch]);
	if (fclose(asmg.file)) { fatal_file_ops(filename); }
}

static void amd64_emmit_inc (const struct asmgen *asmg, const unsigned long group) { fprintf(asmg->file, "\tadd%c\t$%ld, (%%r8)\n", asmg->amd.prefix, group); }

static void amd64_emmit_dec (const struct asmgen *asmg, const unsigned long group) { fprintf(asmg->file, "\tsub%c\t$%ld, (%%r8)\n", asmg->amd.prefix, group); }

static void amd64_emmit_nxt (const struct asmgen *asmg, const unsigned long group) { fprintf(asmg->file, "\taddq\t$%ld, %%r8\n", group * asmg->cellwidth); }

static void amd64_emmit_prv (const struct asmgen *asmg, const unsigned long group) { fprintf(asmg->file, "\tsubq\t$%ld, %%r8\n", group * asmg->cellwidth); }

static void amd64_emmit_out (const struct asmgen *asmg, const unsigned long group)
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

static void amd64_emmit_inp (const struct asmgen *asmg, const unsigned long group)
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

static void amd64_emmit_lbr (const struct asmgen *asmg, const unsigned long branch)
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

static void amd64_emmit_rbr (const struct asmgen *asmg, const unsigned long branch)
{
	static const char *const template =
		"\tjmp\tLB%ld\n"
		"LE%ld:\n";
	fprintf(asmg->file, template, branch, branch);
}

static void arm64_emmit_inc (const struct asmgen *asmg, const unsigned long group)
{
	static const char *const template =
		"\t%s\t%c10, [x9]\n"
		"\tmov\t%c11, #%ld\n"
		"\tadd\t%c10, %c10, %c11\n"
		"\t%s\t%c10, [x9]\n";
	fprintf(
		asmg->file, template, asmg->arm.load, asmg->arm.prefix,
		asmg->arm.prefix, group,
		asmg->arm.prefix, asmg->arm.prefix, asmg->arm.prefix,
		asmg->arm.store, asmg->arm.prefix
	);
}

static void arm64_emmit_dec (const struct asmgen *asmg, const unsigned long group)
{
	static const char *const template =
		"\t%s\t%c10, [x9]\n"
		"\tmov\t%c11, #%ld\n"
		"\tsub\t%c10, %c10, %c11\n"
		"\t%s\t%c10, [x9]\n";
	fprintf(
		asmg->file, template, asmg->arm.load, asmg->arm.prefix,
		asmg->arm.prefix, group,
		asmg->arm.prefix, asmg->arm.prefix, asmg->arm.prefix,
		asmg->arm.store, asmg->arm.prefix
	);
}

static void arm64_emmit_nxt (const struct asmgen *asmg, const unsigned long group)
{
	static const char *const template =
		"\tmov\tx10, #%ld\n"
		"\tadd\tx9, x9, x10\n";
	fprintf(asmg->file, template, group * asmg->cellwidth);
}

static void arm64_emmit_prv (const struct asmgen *asmg, const unsigned long group)
{
	static const char *const template =
		"\tmov\tx10, #%ld\n"
		"\tsub\tx9, x9, x10\n";
	fprintf(asmg->file, template, group * asmg->cellwidth);
}

static void arm64_emmit_out (const struct asmgen *asmg, const unsigned long group)
{
	static const char *const template =
		"\tmov\tx8, #64\n"
		"\tmov\tx0, #1\n"
		"\tmov\tx1, x9\n"
		"\tmov\tx2, #1\n"
		"\tsvc\t#0\n";
	for (unsigned long i = 0; i < group; i++)
	{
		fprintf(asmg->file, "%s", template);
	}
}

static void arm64_emmit_inp (const struct asmgen *asmg, const unsigned long group)
{
	static const char *const template =
		"\tmov\tx8, #63\n"
		"\tmov\tx0, #0\n"
		"\tmov\tx1, x9\n"
		"\tmov\tx2, #1\n"
		"\tsvc\t#0\n";
	for (unsigned long i = 0; i < group; i++)
	{
		fprintf(asmg->file, "%s", template);
	}
}

static void arm64_emmit_lbr (const struct asmgen *asmg, const unsigned long branch)
{
	static const char *template =
		"LB%ld:\n"
		"\t%s\t%c10, [x9]\n"
		"\tcmp\t%c10, #0\n"
		"\tbeq\tLE%d\n";
	fprintf(asmg->file, template, branch, asmg->arm.load, asmg->arm.prefix, asmg->arm.prefix, branch);
}

static void arm64_emmit_rbr (const struct asmgen *asmg, const unsigned long branch)
{
	static const char *const template =
		"\tb\tLB%ld\n"
		"LE%ld:\n";
	fprintf(asmg->file, template, branch, branch);
}
