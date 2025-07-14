/* bc - brainfuck compiler
 * Jul 14, 2025
 * Fatals handler
 */
#ifndef BC_FATAL_H
#define BC_FATAL_H


enum FatalSourceKind
{
	FATAL_SRC_MAX_NESTED_LEVEL = 0,
	FATAL_SRC_PREMATURE_CLOSING,
};

void fatal_file_ops (const char*);
void fatal_memory_ops (const char*);

void fatal_source_fatal (const char*, const unsigned short, const unsigned short, const enum FatalSourceKind);

#endif
