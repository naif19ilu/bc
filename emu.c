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

void emu_emulate (const struct stream *stream, const unsigned int tapeSize, const unsigned char cellSize, const bool safeMode)
{
	struct Memory mem = {
		.at = 0,
		.tapeSize = tapeSize,
		.cellSize = cellSize,
		.safe     = safeMode
	};

	typedef void (*handlerfx_t) (struct token*, struct Memory*);
	handlerfx_t inc, dec;

	switch (cellSize)
	{
		case 1: { mem.memory = (unsigned char*)  calloc(tapeSize, sizeof(unsigned char));  mem.max = UCHAR_MAX; inc = handle_add8 ; dec = handle_dec8 ; break; }
		case 2: { mem.memory = (unsigned short*) calloc(tapeSize, sizeof(unsigned short)); mem.max = USHRT_MAX; inc = handle_add16; dec = handle_dec16; break; }
		case 4: { mem.memory = (unsigned int*)   calloc(tapeSize, sizeof(unsigned int));   mem.max = UINT_MAX;  inc = handle_add32; dec = handle_dec32; break; }
		case 8: { mem.memory = (unsigned long*)  calloc(tapeSize, sizeof(unsigned long));  mem.max = ULONG_MAX; inc = handle_add64; dec = handle_dec64; break; }
	}

	for (size_t i = 0; i < stream->length; i++)
	{
		struct token *t = &stream->stream[i];
		switch (t->meta.mnemonic)
		{
			case '+': inc(t, &mem);         break;
			case '-': dec(t, &mem);         break;
			case '>': handle_next(t, &mem); break;
			case '<': handle_prev(t, &mem); break;
		}
	}
}

inline static void handle_next (struct token *t, struct Memory *mem)
{
	if (mem->safe && ((mem->at + t->groupSize) > mem->tapeSize))
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_NEXT_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	mem->at += t->groupSize;
}

inline static void handle_prev (struct token *t, struct Memory *mem)
{
	if (mem->safe && ((((signed int) mem->at) - t->groupSize) < 0))
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_PREV_UNDRFLOW, FATAL_ISNT_MULTIPLE);
	}
	mem->at -= t->groupSize;
}

inline static void handle_add8 (struct token *t, struct Memory *mem)
{
	unsigned char *byte = &(((unsigned char*) mem->memory)[mem->at]);
	unsigned long a     = (unsigned long) *byte;

	if (mem->safe && (a + (unsigned long) t->groupSize) > mem->max)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_INCS_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	*byte += mem->tapeSize;
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
	unsigned long a     = (unsigned long) *word;

	if (mem->safe && (a + (unsigned long) t->groupSize) > mem->max)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_INCS_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	*word += mem->tapeSize;
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

	if (mem->safe && (a + (unsigned long) t->groupSize) > mem->max)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_INCS_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	*longg += mem->tapeSize;
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
	unsigned int *quad = &(((unsigned int*) mem->memory)[mem->at]);
	unsigned long a     = (unsigned long) *quad;

	if (mem->safe && (a + (unsigned long) t->groupSize) > mem->max)
	{
		fatal_source_fatal(FATAL_BREAKDOWN_TOKEN(t), FATAL_SRC_SAFE_MODE_INCS_OVERFLOW, FATAL_ISNT_MULTIPLE);
	}
	*quad += mem->tapeSize;
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
