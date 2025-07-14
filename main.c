/* bc - brainfuck compiler
 * Jul 13, 2025
 * Main file
 */
#include "cxa.h"
#include "fatal.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct bc
{
	struct
	{
		char          *compile;
		char          *output;
		char          *source;
		unsigned int  tapeSize;
		unsigned char cellSize;
		bool          safeMode;
	} args;

	char   *source;
	size_t length;
};

static size_t read_file (const char*, char**);

int main (int argc, char **argv)
{
	struct bc bc =
	{
		.args.output   = "a.out",
		.args.source   = "a.s",
		.args.tapeSize = 30000,
		.args.cellSize = 1
	};

	struct CxaFlag flags[] =
	{
		CXA_SET_STR("compile", "source to be compiled",                   &bc.args.compile,  CXA_FLAG_TAKER_YES, 'c'),
		CXA_SET_STR("output",  "name for the objfile (a.out default)",    &bc.args.output,   CXA_FLAG_TAKER_MAY, 'o'),
		CXA_SET_STR("source",  "produce asm code no ELF (a.out default)", &bc.args.source,   CXA_FLAG_TAKER_MAY, 'S'),
		CXA_SET_INT("tapesz",  "tape size (30000 default)",               &bc.args.tapeSize, CXA_FLAG_TAKER_MAY, 'T'),
		CXA_SET_CHR("cellsz",  "cell size (1 B default: 1,2,4,8 B)",      &bc.args.cellSize, CXA_FLAG_TAKER_MAY, 'C'),
		CXA_SET_CHR("usage",   "displays this message",                   NULL,              CXA_FLAG_TAKER_NON, 'u'),
		CXA_SET_CHR("safe",    "enable safe mode",                        NULL,              CXA_FLAG_TAKER_NON, 's'),
		CXA_SET_END
	};

	(void) cxa_execute((unsigned char) argc, argv, flags, "bc");

	if (!bc.args.compile || (flags[5].meta & CXA_FLAG_SEEN_MASK))
	{
		cxa_print_usage("brainfuck compiler - x86_64", flags);
		return 0;
	}

	bc.args.safeMode = flags[6].meta & CXA_FLAG_SEEN_MASK;
	bc.length = read_file(bc.args.compile, &bc.source);

	return 0;
}

static size_t read_file (const char *filename, char **source)
{
	FILE *file = fopen(filename, "r");
	if (!file)
	{
		fatal_file_ops(filename);
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	*source = (char*) calloc(size + 1, sizeof(char));
	if (*source == NULL)
	{
		fatal_memory_ops("cannot reserve space for reading the source code");
	}

	fread(*source, 1, size, file);
	if (ferror(file))
	{
		fatal_file_ops(filename);
	}

	fclose(file);
	return size;
}
