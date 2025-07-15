/* bc - brainfuck compiler
 * Jul 15, 2025 Happy birthday!!
 * Assembly generator (-S)
 */
#include "asm.h"
#include "fatal.h"

void asm_gen_asm (const struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned int cellSize)
{
	FILE *file = fopen(filename, "w");
	if (!file)
	{
		fatal_file_ops(filename);
	}

	const char *const template =
		".section .text\n"
		".globl _start\n"
		"_start:\n"
		"\tpushq\t%%rbp\n"
		"\tmovq\t%%rsp, %%rbp\n"
		"\tsubq\t$%d, %rsp\n"
		"\tleaq\t(%%rbp), %%r8\n";

	fprintf(file, template, tapeSize * cellSize);
	fclose(file);
}
