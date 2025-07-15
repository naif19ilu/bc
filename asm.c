/* bc - brainfuck compiler
 * Jul 15, 2025 Happy birthday!!
 * Assembly generator (-S)
 */
#include "asm.h"
#include "fatal.h"

inline static void produce_input_syscall (FILE*, const unsigned long);
inline static void produce_output_syscall (FILE*, const unsigned long);

inline static void produce_loop_closing (FILE*, const unsigned long, const char);

void asm_gen_asm (const struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned int cellSize)
{
	FILE *file = fopen(filename, "w");
	if (!file)
	{
		fatal_file_ops(filename);
	}

	char prefix = 'b';
	switch (cellSize)
	{
		case 2: prefix = 'w'; break;
		case 4: prefix = 'l'; break;
		case 8: prefix = 'q'; break;
	}

	static const char *const template =
		".section .text\n"
		".globl _start\n"
		"_start:\n"
		"\tpushq\t%rbp\n"
		"\tmovq\t%rsp, %rbp\n"
		"\tsubq\t$%d, %rsp\n"
		"\tleaq\t(%rbp), %r8\n";
	fprintf(file, template, tapeSize * cellSize);

	for (size_t i = 0; i < stream->length; i++)
	{
		const struct token *t = &stream->stream[i];
		switch (t->meta.mnemonic)
		{
			case '+': fprintf(file, "\tadd%c\t$%ld, (%%r8)\n", prefix, t->groupSize); break;
			case '-': fprintf(file, "\tsub%c\t$%ld, (%%r8)\n", prefix, t->groupSize); break;
			case '>': fprintf(file, "\taddq\t$%ld, %%r8\n",            t->groupSize); break;
			case '<': fprintf(file, "\tsubq\t$%ld, %%r8\n",            t->groupSize); break;
			case ',': produce_input_syscall(file, t->groupSize);                      break;
			case '.': produce_output_syscall(file, t->groupSize);                     break;
			case '[': fprintf(file, "LB%ld:\n", t->parnerPosition);                   break;
			case ']': produce_loop_closing(file, t->parnerPosition, prefix);          break;
		}
	}

	static const char *const leave =
		"\tmovq\t$60, %rax\n"
		"\tmovq\t$0, %rdi\n"
		"\tsyscall\n";

	fprintf(file, "%s", leave);
	fclose(file);
}

inline static void produce_input_syscall (FILE *file, const unsigned long times)
{
	static const char *const template =
		"\tmovq\t$0, %rax\n"
		"\tmovq\t$0, %rdi\n"
		"\tmovq\t$1, %rdx\n"
		"\tmovq\t%r8, %rsi\n"
		"\tsyscall\n";
	for (unsigned long i = 0; i < times; i++)
	{
		fprintf(file, "%s", template);
	}
}

inline static void produce_output_syscall (FILE *file, const unsigned long times)
{
	static const char *const template =
		"\tmovq\t$1, %rax\n"
		"\tmovq\t$1, %rdi\n"
		"\tmovq\t$1, %rdx\n"
		"\tmovq\t%r8, %rsi\n"
		"\tsyscall\n";
	for (unsigned long i = 0; i < times; i++)
	{
		fprintf(file, "%s", template);
	}
}

inline static void produce_loop_closing (FILE *file, const unsigned long branch, const char prefix)
{
	if (prefix == 'q')
	{
		static const char *const template =
			"\tmovq\t(%r8), %r9\n"
			"\tcmpq\t$0, %r9\n"
			"\tjne\tLB%ld\n";
		fprintf(file, template, branch);
		return;
	}

	char regprefx = prefix;
	if (prefix == 'l') { regprefx = 'd'; }

	static const char *const template =
		"\tmovzbl\t(%r8), %r9d\n"
		"\tcmp%c\t$0, %r9%c\n"
		"\tjne\tLB%ld\n";

	fprintf(file, template, prefix, regprefx, branch);
	return;
}
