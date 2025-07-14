/* bc - brainfuck compiler
 * Jul 14, 2025
 * Fatals handler
 */
#ifndef BC_FATAL_H
#define BC_FATAL_H

#define FATAL_BREAKDOWN_TOKEN(t)   t->meta.context, t->meta.numline, t->meta.offline

enum FatalSourceKind
{
	FATAL_SRC_MAX_NESTED_LEVEL = 0,
	FATAL_SRC_PREMATURE_OPENING,
	FATAL_SRC_UNMATCHED_OPEN,

	FATAL_SRC_SAFE_MODE_NEXT_OVERFLOW,
	FATAL_SRC_SAFE_MODE_PREV_UNDRFLOW,
};

enum FatalIsMultiple
{
	FATAL_IS_MULTIPLE,
	FATAL_ISNT_MULTIPLE,
};

void fatal_file_ops (const char*);
void fatal_memory_ops (const char*);

void fatal_source_fatal (const char*, const unsigned short, const unsigned short, const enum FatalSourceKind, const enum FatalIsMultiple);

#endif
