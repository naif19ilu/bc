/* bc - brainfuck compiler
 * Jul 14, 2025
 * Definitions file
 */
#ifndef BC_H
#define BC_H

#define BC_DEFAULT_o   "a.out"
#define BC_DEFAULT_S   "a.s"
#define BC_DEFAULT_T   30000
#define BC_DEFAULT_C   1
#define BC_DEFAULT_O   0
#define BC_DEFAULT_d   100
#define BC_DEFAULT_g   10

#define STREAM_GROWTH_FACTOR     128
#define OPENLOOP_STACK_MAX_CAP   256
#define CHECK_POINTER(ptr, a)    do { if (ptr) break; fatal_memory_ops(a); } while (0)

#include <stdio.h>
#include <stdbool.h>

enum arch
{
	ARCH_AMD64 = 0,
	ARCH_ARM64 = 1,
};

struct token
{
	unsigned long groupSize;
	unsigned long nolbl;
	struct
	{
		char           *context;
		unsigned short numline;
		unsigned short offline;
		char           mnemonic;
	} meta;
};

struct stream
{
	struct token  *stream;
	size_t        length;
	size_t        capacity;
	unsigned long nonested;
};

struct bc
{
	struct stream stream;
	char          *source;
	size_t        length;
	struct
	{
		unsigned int   offset;
		unsigned int   tapesz;
		unsigned int   display;
		unsigned int   group;
		unsigned char  cellsz;
		char           *compile;
		char           *output;
		char           *source;
		bool           safeMode;
		bool           emulate;
		enum arch      arch;
	} args;
};

#endif
