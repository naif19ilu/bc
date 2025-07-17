/* bc - brainfuck compiler
 * Jul 13, 2025
 * Main file
 */
#include "asm.h"
#include "emu.h"
#include "cxa.h"
#include "fatal.h"
#include "lexpa.h"

#include <stdlib.h>
#include <string.h>

static size_t read_file (const char*, char**);
static void check_arguments (struct bc*);

int main (int argc, char **argv)
{
	struct bc bc =
	{
		.args.output    = BC_DEFAULT_o,
		.args.source    = BC_DEFAULT_S,
		.args.tapeSize  = BC_DEFAULT_T,
		.args.cellSize  = BC_DEFAULT_C,
		.args.offset    = BC_DEFAULT_O,
		.args.display   = BC_DEFAULT_d,
		.args.group     = BC_DEFAULT_g,
	};

	char *arch = "amd64";
	struct CxaFlag flags[] =
	{
		CXA_SET_STR("compile", "source to be compiled",                                         &bc.args.compile,  CXA_FLAG_TAKER_YES, 'c'),
		CXA_SET_STR("output",  "name for the objfile (a.out default)",                          &bc.args.output,   CXA_FLAG_TAKER_MAY, 'o'),
		CXA_SET_STR("source",  "produce asm code, no ELF (a.out default)",                      &bc.args.source,   CXA_FLAG_TAKER_MAY, 'S'),
		CXA_SET_INT("tape",    "tape size (30000 default)",                                     &bc.args.tapeSize, CXA_FLAG_TAKER_MAY, 'T'),
		CXA_SET_INT("cell",    "cell size (1 B default. 1,2,4,8 B)",                            &bc.args.cellSize, CXA_FLAG_TAKER_MAY, 'C'),
		CXA_SET_CHR("usage",   "displays this message",                                         NULL,              CXA_FLAG_TAKER_NON, 'u'),
		CXA_SET_CHR("safe",    "enables safe mode",                                             NULL,              CXA_FLAG_TAKER_NON, 's'),
		CXA_SET_CHR("emu-mem", "emulates memory (disabled by default)",                         NULL,              CXA_FLAG_TAKER_NON, 'E'),
		CXA_SET_INT("offset",  "print emulated memory from <offset> (0 default)",               &bc.args.offset,   CXA_FLAG_TAKER_MAY, 'O'),
		CXA_SET_INT("display", "number of cells to display from emulated memory (100 default)", &bc.args.display,  CXA_FLAG_TAKER_MAY, 'd'),
		CXA_SET_INT("group",   "number of columns to display when emu-mem (10 default)",        &bc.args.group,    CXA_FLAG_TAKER_MAY, 'g'),
		CXA_SET_STR("arch",    "target archquitecture (amd64 default, amd64 | arm64)",          &arch,             CXA_FLAG_TAKER_MAY, 'A'),
		CXA_SET_END
	};

	cxa_clean(cxa_execute((unsigned char) argc, argv, flags, "bc"));
	bc.args.arch = (strncmp(arch, "arm64", 5) == 0 ? ARCH_ARM64 : ARCH_AMD64);

	if (!bc.args.compile || (flags[5].meta & CXA_FLAG_SEEN_MASK))
	{
		cxa_print_usage("brainfuck compiler - x86_64", flags);
		return 0;
	}

	bc.args.safeMode = flags[6].meta & CXA_FLAG_SEEN_MASK;
	bc.args.emulate  = flags[7].meta & CXA_FLAG_SEEN_MASK;
	check_arguments(&bc);

	bc.length = read_file(bc.args.compile, &bc.source);
	lexpa_lex_n_parse(bc.source, bc.length, &bc.stream);

	if (bc.args.emulate || bc.args.safeMode)
	{
		emu_emulate(&bc.stream, bc.args.tapeSize, bc.args.cellSize, bc.args.safeMode, bc.args.offset, bc.args.display, bc.args.group);
		return 0;
	}

	if (flags[2].meta & CXA_FLAG_SEEN_MASK)
	{
		asm_gen_asm(&bc.stream, bc.args.source, bc.args.tapeSize, bc.args.cellSize, bc.args.arch);
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

static void check_arguments (struct bc *bc)
{
	static const unsigned short cellkeyMask = ((1 << 1) | (1 << 2) | (1 << 4) | (1 << 8));

	if ((cellkeyMask & (1 << bc->args.cellSize)) == 0)
	{
		fatal_nonfatal_warn(FATAL_WARN_INVALID_C, bc->args.cellSize, BC_DEFAULT_C);
		bc->args.cellSize = BC_DEFAULT_C;
	}
	if (bc->args.tapeSize < 30000)
	{
		fatal_nonfatal_warn(FATAL_WARN_INVALID_T, bc->args.tapeSize, BC_DEFAULT_T);
		bc->args.tapeSize = BC_DEFAULT_T;
	}

	if (bc->args.safeMode)
	{
		return;
	}

	if (bc->args.offset > bc->args.tapeSize)
	{
		fatal_nonfatal_warn(FATAL_WARN_INVALID_OT, bc->args.offset, bc->args.tapeSize);
		bc->args.tapeSize = BC_DEFAULT_T;
		bc->args.offset   = BC_DEFAULT_O;
	}
	if (bc->args.display > bc->args.tapeSize || ((bc->args.display + bc->args.offset) > bc->args.tapeSize))
	{
		fatal_nonfatal_warn(FATAL_WARN_INVALID_dT, bc->args.display, bc->args.tapeSize);
		bc->args.tapeSize = BC_DEFAULT_T;
		bc->args.display  = BC_DEFAULT_d;
	}
	if (bc->args.group == 0)
	{
		fatal_nonfatal_warn(FATAL_WARN_INVALID_g, bc->args.group, BC_DEFAULT_g);
		bc->args.group  = BC_DEFAULT_g;
	}
}
