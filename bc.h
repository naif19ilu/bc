/* bc - brainfuck compiler
 * Jul 14, 2025
 * Definitions file
 */
#ifndef BC_H
#define BC_H

#define STREAM_GROWTH_FACTOR     128
#define OPENLOOP_STACK_MAX_CAP   256
#define CHECK_POINTER(ptr, a)    do { if (ptr) break; fatal_memory_ops(a); } while (0)

#include <stdio.h>
#include <stdbool.h>

struct token
{
	unsigned long ELFoffset;
	short         parnerPosition;
	short         groupSize;

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
	struct token *stream;
	size_t        length;
	size_t        capacity;
};

struct bc
{
	struct stream stream;
	char          *source;
	size_t        length;
	struct
	{
		unsigned int   offset;
		unsigned int   tapeSize;
		unsigned short display;
		unsigned char  cellSize;
		char           *compile;
		char           *output;
		char           *source;
		bool           safeMode;
		bool           emulate;
	} args;
};

#endif
