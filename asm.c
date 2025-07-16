/* bc - brainfuck compiler
 * Jul 15, 2025 Happy birthday!!
 * Assembly generator (-S)
 */
#include "asm.h"
#include "fatal.h"

struct asmgen
{
	FILE          *file;
	char          *x64regName;
	char          x64prefix;
	unsigned char factor;
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
};

static const char *const Footers[] =
{
	"\tmovq\t$60, %rax\n"
	"\tmovq\t$0, %rdi\n"
	"\tsyscall\n",

};

/*  ________________________________________
 * < all this is for AMD64 code generation! >
 *  ----------------------------------------
 *    \
 *     \
 *         .--.
 *        |o_o |
 *        |:_/ |
 *       //   \ \
 *      (|     | )
 *     /'\_   _/`\
 *     \___)=(___/
 */
static void get_x64_prefixes (struct asmgen *asmg, const unsigned char cellSize)
{
	switch (cellSize)
	{
		case 1: { asmg->x64regName = "al" ; asmg->x64prefix = 'b'; break; }
		case 2: { asmg->x64regName = "ax" ; asmg->x64prefix = 'w'; break; }
		case 4: { asmg->x64regName = "eax"; asmg->x64prefix = 'l'; break; }
		case 8: { asmg->x64regName = "rax"; asmg->x64prefix = 'q'; break; }
	}
	asmg->factor = cellSize;
}

inline static void x64_emmit_inc (const struct asmgen *asmg, const unsigned long group)
{
	fprintf(asmg->file, "\tadd%c\t$%ld, (%%r8)\n", asmg->x64prefix, group);
}

inline static void x64_emmit_dec (const struct asmgen *asmg, const unsigned long group)
{
	fprintf(asmg->file, "\tsub%c\t$%ld, (%%r8)\n", asmg->x64prefix, group);
}

inline static void x64_emmit_nxt (const struct asmgen *asmg, const unsigned long group)
{
	fprintf(asmg->file, "\taddq\t$%ld, %%r8\n", group * asmg->factor);
}

inline static void x64_emmit_prv (const struct asmgen *asmg, const unsigned long group)
{
	fprintf(asmg->file, "\tsubq\t$%ld, %%r8\n", group * asmg->factor);
}

inline static void x64_emmit_out (const struct asmgen *asmg, const unsigned long group)
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

inline static void x64_emmit_inp (const struct asmgen *asmg, const unsigned long group)
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

inline static void x64_emmit_lbr (const struct asmgen *asmg, const unsigned long branch)
{
	if (asmg->x64prefix == 'q')
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
	fprintf(asmg->file, template, branch, asmg->x64prefix, asmg->x64regName, branch);
}

inline static void x64_emmit_rbr (const struct asmgen *asmg, const unsigned long branch)
{
	static const char *const template =
		"\tjmp\tLB%ld\n"
		"LE%ld:\n";
	fprintf(asmg->file, template, branch, branch);
}

void asm_gen_asm (const struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned char cellSize, const enum arch arch)
{
	struct asmgen asmg = { .file = fopen(filename, "w") };
	if (!asmg.file)
	{
		fatal_file_ops(filename);
	}

	get_x64_prefixes(&asmg, cellSize);
	fprintf(asmg.file, *Headers, (unsigned long) (tapeSize * cellSize));

	for (size_t i = 0; i < stream->length; i++)
	{
		const struct token *token = &stream->stream[i];
		switch (token->meta.mnemonic)
		{
			case '+': x64_emmit_inc(&asmg, token->groupSize);      break;
			case '-': x64_emmit_dec(&asmg, token->groupSize);      break;
			case '>': x64_emmit_nxt(&asmg, token->groupSize);      break;
			case '<': x64_emmit_prv(&asmg, token->groupSize);      break;
			case '.': x64_emmit_out(&asmg, token->groupSize);      break;
			case ',': x64_emmit_inp(&asmg, token->groupSize);      break;
			case '[': x64_emmit_lbr(&asmg, token->parnerPosition); break;
			case ']': x64_emmit_rbr(&asmg, token->parnerPosition); break;
		}
	}

	fprintf(asmg.file, "%s", *Footers);
	fclose(asmg.file);
}
