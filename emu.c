/* bc - brainfuck compiler
 * Jul 14, 2025
 * Memory (tape) emulator
 */
#include "emu.h"
#include "fatal.h"

#include <stdlib.h>
#include <limits.h>

struct Memory
{
	void          *memory;
	unsigned long max;
	unsigned int  at;
	unsigned int  tapeSize;
	unsigned char cellSize;
	bool          safe;
};

static void handle_next (struct token*, struct Memory*);
static void handle_prev (struct token*, struct Memory*);

void emu_emulate (const struct stream *stream, const unsigned int tapeSize, const unsigned char cellSize, const bool safeMode)
{
	struct Memory mem = {
		.at = 0,
		.tapeSize = tapeSize,
		.cellSize = cellSize,
		.safe     = safeMode
	};

	switch (cellSize)
	{
		case 1: { mem.memory = (unsigned char*)  calloc(tapeSize, sizeof(unsigned char));  mem.max = UCHAR_MAX; break; }
		case 2: { mem.memory = (unsigned short*) calloc(tapeSize, sizeof(unsigned short)); mem.max = USHRT_MAX; break; }
		case 4: { mem.memory = (unsigned int*)   calloc(tapeSize, sizeof(unsigned int));   mem.max = UINT_MAX;  break; }
		case 8: { mem.memory = (unsigned long*)  calloc(tapeSize, sizeof(unsigned long));  mem.max = ULONG_MAX; break; }
	}

	for (size_t i = 0; i < stream->length; i++)
	{
		struct token *t = &stream->stream[i];
		switch (t->meta.mnemonic)
		{
			case '+': break;
			case '-': break;
			case '>': handle_next(t, &mem); break;
			case '<': handle_prev(t, &mem); break;
		}
	}
}

static void handle_next (struct token *t, struct Memory *mem)
{
	if (mem->safe && ((mem->at + t->groupSize) >= mem->tapeSize))
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_NEXT_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	mem->at += t->groupSize;
}


static void handle_prev (struct token *t, struct Memory *mem)
{
	if (mem->safe && ((((signed int) mem->at) - t->groupSize) < 0))
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_PREV_UNDRFLOW, FATAL_ISNT_MULTIPLE);
	}
	mem->at += t->groupSize;
}

