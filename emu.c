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

inline static void handle_next (struct token*, struct Memory*);
inline static void handle_prev (struct token*, struct Memory*);

inline static void handle_add8 (struct token*, struct Memory*);
inline static void handle_dec8 (struct token*, struct Memory*);

inline static void handle_add16 (struct token*, struct Memory*);
inline static void handle_dec16 (struct token*, struct Memory*);

inline static void handle_add32 (struct token*, struct Memory*);
inline static void handle_dec32 (struct token*, struct Memory*);

inline static void handle_add64 (struct token*, struct Memory*);
inline static void handle_dec64 (struct token*, struct Memory*);

inline static void display_8  (struct Memory*, const unsigned int, const unsigned int, const unsigned int);
inline static void display_16 (struct Memory*, const unsigned int, const unsigned int, const unsigned int);

inline static void display_32 (struct Memory*, const unsigned int, const unsigned int, const unsigned int);
inline static void display_64 (struct Memory*, const unsigned int, const unsigned int, const unsigned int);

void emu_emulate (const struct stream *stream, const unsigned int tapeSize, const unsigned char cellSize, const bool safeMode, const unsigned int offset, const unsigned int display, const unsigned int group)
{
	struct Memory mem = {
		.at = 0,
		.tapeSize = tapeSize,
		.cellSize = cellSize,
		.safe     = safeMode
	};

	typedef void (*incdec_t) (struct token*, struct Memory*);
	typedef void (*display_t) (struct Memory*, const unsigned int, const unsigned int, const unsigned int);

	incdec_t inc, dec;
	display_t dis;

	switch (cellSize)
	{
		case 1: { mem.memory = (unsigned char*)  calloc(tapeSize, sizeof(unsigned char));  mem.max = UCHAR_MAX; inc = handle_add8 ; dec = handle_dec8 ; dis = display_8 ; break; }
		case 2: { mem.memory = (unsigned short*) calloc(tapeSize, sizeof(unsigned short)); mem.max = USHRT_MAX; inc = handle_add16; dec = handle_dec16; dis = display_16; break; }
		case 4: { mem.memory = (unsigned int*)   calloc(tapeSize, sizeof(unsigned int));   mem.max = UINT_MAX;  inc = handle_add32; dec = handle_dec32; dis = display_32; break; }
		case 8: { mem.memory = (unsigned long*)  calloc(tapeSize, sizeof(unsigned long));  mem.max = ULONG_MAX; inc = handle_add64; dec = handle_dec64; dis = display_64; break; }
	}

	CHECK_POINTER(mem.memory, "reserving space to emulate memory");

	for (size_t i = 0; i < stream->length; i++)
	{
		struct token *t = &stream->stream[i];
		switch (t->meta.mnemonic)
		{
			case '+': inc(t, &mem);         break;
			case '-': dec(t, &mem);         break;
			case '>': handle_next(t, &mem); break;
			case '<': handle_prev(t, &mem); break;
			case '@':
			{
				if (safeMode) continue;
				printf("debbug at %ld chunk-instruction from %d cell:\n", i - 1, offset);

				dis(&mem, offset, display, group);
				printf("\n\n");
				break;
			}
		}
	}

	if (safeMode)
	{
		puts("All is within bounds :) good job!");
	}
}

inline static void handle_next (struct token *t, struct Memory *mem)
{
	if (mem->safe && ((mem->at + t->groupSize) > mem->tapeSize))
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_NEXT_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	mem->at += (unsigned int) t->groupSize;
}

inline static void handle_prev (struct token *t, struct Memory *mem)
{
	const signed long help = mem->at - t->groupSize;
	if (mem->safe && (help < 0))
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_PREV_UNDRFLOW, FATAL_ISNT_MULTIPLE);
	}
	mem->at -= (unsigned int) t->groupSize;
}

inline static void handle_add8 (struct token *t, struct Memory *mem)
{
	unsigned char *byte = &(((unsigned char*) mem->memory)[mem->at]);
	unsigned long a     = (unsigned long) *byte;

	if (mem->safe && (a + t->groupSize) > mem->max)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_INCS_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	*byte += (unsigned char) t->groupSize;
}

inline static void handle_dec8 (struct token *t, struct Memory *mem)
{
	unsigned char *byte = &(((unsigned char*) mem->memory)[mem->at]);
	if (mem->safe && *byte == 0)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_DECS_UNDRFLOW, FATAL_ISNT_MULTIPLE);
	}
	*byte -= (unsigned char) t->groupSize;
}

inline static void handle_add16 (struct token *t, struct Memory *mem)
{
	unsigned short *word = &(((unsigned short*) mem->memory)[mem->at]);
	unsigned long a      = (unsigned long) *word;

	if (mem->safe && (a + t->groupSize) > mem->max)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_INCS_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	*word += (unsigned short) t->groupSize;
}

inline static void handle_dec16 (struct token *t, struct Memory *mem)
{
	unsigned short *word = &(((unsigned short*) mem->memory)[mem->at]);

	if (mem->safe && *word == 0)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_DECS_UNDRFLOW, FATAL_ISNT_MULTIPLE);
	}
	*word -= (unsigned short) t->groupSize;
}

inline static void handle_add32 (struct token *t, struct Memory *mem)
{
	unsigned int *longg = &(((unsigned int*) mem->memory)[mem->at]);
	unsigned long a     = (unsigned long) *longg;

	if (mem->safe && (a + t->groupSize) > mem->max)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_INCS_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	*longg += (unsigned int) t->groupSize;
}

inline static void handle_dec32 (struct token *t, struct Memory *mem)
{
	unsigned int *longg = &(((unsigned int*) mem->memory)[mem->at]);

	if (mem->safe && *longg == 0)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_DECS_UNDRFLOW, FATAL_ISNT_MULTIPLE);
	}
	*longg -= (unsigned int) t->groupSize;
}

inline static void handle_add64 (struct token *t, struct Memory *mem)
{
	unsigned long *quad = &(((unsigned long*) mem->memory)[mem->at]);
	unsigned long a     = *quad;

	if (mem->safe && (a + t->groupSize) > mem->max)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_INCS_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	*quad += (unsigned long) t->groupSize;
}

inline static void handle_dec64 (struct token *t, struct Memory *mem)
{
	unsigned long *quad = &(((unsigned long*) mem->memory)[mem->at]);

	if (mem->safe && *quad == 0)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_DECS_UNDRFLOW, FATAL_ISNT_MULTIPLE);
	}
	*quad -= (unsigned long) t->groupSize;
}

inline static void display_8  (struct Memory *mem, const unsigned int off, const unsigned int dis, const unsigned int group)
{
	for (unsigned int i = off, j = 0; i < dis + off; j++)
	{
		if ((j % group) == 0 && j)
		{
			putchar(10);
		}
		printf("  0x%.2x", ((unsigned char*) mem->memory)[i++]);
	}
}

inline static void display_16 (struct Memory *mem, const unsigned int off, const unsigned int dis, const unsigned int group)
{
	for (unsigned int i = off, j = 0; i < dis + off; j++)
	{
		if ((j % group) == 0 && j)
		{
			putchar(10);
		}
		printf("  0x%.4x", ((unsigned short*) mem->memory)[i++]);
	}
}

inline static void display_32 (struct Memory *mem, const unsigned int off, const unsigned int dis, const unsigned int group)
{
	for (unsigned int i = off, j = 0; i < dis + off; j++)
	{
		if ((j % group) == 0 && j)
		{
			putchar(10);
		}
		printf("  0x%.8x", ((unsigned int*) mem->memory)[i++]);
	}
}

inline static void display_64 (struct Memory *mem, const unsigned int off, const unsigned int dis, const unsigned int group)
{
	for (unsigned int i = off, j = 0; i < dis + off; j++)
	{
		if ((j % group) == 0 && j)
		{
			putchar(10);
		}
		printf("  0x%.16lx", ((unsigned long*) mem->memory)[i++]);
	}
}
