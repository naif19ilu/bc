/* bc - brainfuck compiler
 * Jul 13, 2025
 * Main file
 */
#include "emu.h"
#include "cxa.h"
#include "fatal.h"
#include "lexpa.h"

#include <stdlib.h>

static size_t read_file (const char*, char**);

int main (int argc, char **argv)
{
	struct bc bc =
	{
		.args.output    = "a.out",
		.args.source    = "a.s",
		.args.tapeSize  = 30000,
		.args.cellSize  = 1,
		.args.offset    = 0,
		.args.display   = 100
	};

	struct CxaFlag flags[] =
	{
		CXA_SET_STR("compile", "source to be compiled",                                         &bc.args.compile,  CXA_FLAG_TAKER_YES, 'c'),
		CXA_SET_STR("output",  "name for the objfile (a.out default)",                          &bc.args.output,   CXA_FLAG_TAKER_MAY, 'o'),
		CXA_SET_STR("source",  "produce asm code, no ELF (a.out default)",                      &bc.args.source,   CXA_FLAG_TAKER_MAY, 'S'),
		CXA_SET_INT("tapesz",  "tape size (30000 default)",                                     &bc.args.tapeSize, CXA_FLAG_TAKER_MAY, 'T'),
		CXA_SET_INT("cellsz",  "cell size (1 B default. 1,2,4,8 B)",                            &bc.args.cellSize, CXA_FLAG_TAKER_MAY, 'C'),
		CXA_SET_CHR("usage",   "displays this message",                                         NULL,              CXA_FLAG_TAKER_NON, 'u'),
		CXA_SET_CHR("safe",    "enables safe mode",                                             NULL,              CXA_FLAG_TAKER_NON, 's'),
		CXA_SET_CHR("emu-mem", "emulates memory (disabled by default)",                         NULL,              CXA_FLAG_TAKER_NON, 'E'),
		CXA_SET_INT("offset",  "print emulated memory from <offset> (0 default)",               &bc.args.offset,   CXA_FLAG_TAKER_MAY, 'O'),
		CXA_SET_INT("display", "number of cells to display from emulated memory (100 default)", &bc.args.display,  CXA_FLAG_TAKER_MAY, 'd'),

		CXA_SET_END
	};

	cxa_clean(cxa_execute((unsigned char) argc, argv, flags, "bc"));

	if (!bc.args.compile || (flags[5].meta & CXA_FLAG_SEEN_MASK))
	{
		cxa_print_usage("brainfuck compiler - x86_64", flags);
		return 0;
	}

	bc.args.safeMode = flags[6].meta & CXA_FLAG_SEEN_MASK;
	bc.args.emulate  = flags[7].meta & CXA_FLAG_SEEN_MASK;

	if (bc.args.cellSize != 1 && bc.args.cellSize != 2 && bc.args.cellSize != 4 && bc.args.cellSize != 8)
	{
		bc.args.cellSize = 1;
	}
	if (bc.args.tapeSize < 30000)
	{
		bc.args.tapeSize = 30000;
	}

	bc.length = read_file(bc.args.compile, &bc.source);
	lexpa_lex_n_parse(bc.source, bc.length, &bc.stream);

	if (bc.args.emulate || bc.args.safeMode)
	{
		emu_emulate(&bc.stream, bc.args.tapeSize, bc.args.cellSize, bc.args.safeMode);
		return 0;
	}
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
	CHECK_POINTER(*source, "reserving space for reading the source code");

	fread(*source, 1, size, file);
	if (ferror(file))
	{
		fatal_file_ops(filename);
	}

	fclose(file);
	return size;
}
