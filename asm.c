/* bc - brainfuck compiler
 * Jul 15, 2025 Happy birthday!!
 * Assembly generator (-S)
 */
#include "asm.h"
#include "fatal.h"

struct asmgen
{
	FILE *file;
	char *x64regName;
	char x64prefix;
};

static const char *const Header =
	".section .bss\n"
	"\tmemory: .zero %ld\n"
	".section .text\n"
	".globl _start\n"
	"_start:\n"
	"\tleaq\tmemory(%rip), %rax\n"
	"\tmovq\t%rax, %r8\n";

static const char *const Footer =
	"\tmovq\t$60, %rax\n"
	"\tmovq\t$0, %rdi\n"
	"\tsyscall\n";

static void get_x64_prefixes (struct asmgen*, const unsigned char);
inline static void x64_emmit_inc (const struct asmgen*, const unsigned long);
inline static void x64_emmit_dec (const struct asmgen*, const unsigned long);
inline static void x64_emmit_nxt (const struct asmgen*, const unsigned long);
inline static void x64_emmit_prv (const struct asmgen*, const unsigned long);
inline static void x64_emmit_out (const struct asmgen*, const unsigned long);
inline static void x64_emmit_inp (const struct asmgen*, const unsigned long);
inline static void x64_emmit_lbr (const struct asmgen*, const unsigned long);
inline static void x64_emmit_rbr (const struct asmgen*, const unsigned long);

void asm_gen_asm (const struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned char cellSize)
{
	struct asmgen asmg = { .file = fopen(filename, "w") };
	if (!asmg.file)
	{
		fatal_file_ops(filename);
	}

	get_x64_prefixes(&asmg, cellSize);
	fprintf(asmg.file, Header, (unsigned long) (tapeSize * cellSize));

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

	fprintf(asmg.file, "%s", Footer);
	fclose(asmg.file);
}

static void get_x64_prefixes (struct asmgen *asmg, const unsigned char cellSize)
{
	switch (cellSize)
	{
		case 1: { asmg->x64regName = "al" ; asmg->x64prefix = 'b'; break; }
		case 2: { asmg->x64regName = "ax" ; asmg->x64prefix = 'w'; break; }
		case 4: { asmg->x64regName = "eax"; asmg->x64prefix = 'l'; break; }
		case 8: { asmg->x64regName = "rax"; asmg->x64prefix = 'q'; break; }
	}
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
	fprintf(asmg->file, "\taddq\t$%ld, %%r8\n", group);
}

inline static void x64_emmit_prv (const struct asmgen *asmg, const unsigned long group)
{
	fprintf(asmg->file, "\tsubq\t$%ld, %%r8\n", group);
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
	static const char *const template =
		"LB%ld:\n"
		"\tmovzbl\t(%r8), %%eax\n"
		"\tcmpl\t$0, %%eax\n"
		"\tje\tLE%ld\n";
	fprintf(asmg->file, template, branch, branch);
}

inline static void x64_emmit_rbr (const struct asmgen *asmg, const unsigned long branch)
{
	static const char *const template =
		"\tjmp\tLB%ld\n"
		"LE%ld:\n";
	fprintf(asmg->file, template, branch, branch);
}
