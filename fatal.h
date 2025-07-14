/* bc - brainfuck compiler
 * Jul 14, 2025
 * Fatals handler
 */
#ifndef BC_FATAL_H
#define BC_FATAL_H

void fatal_file_ops (const char*);
void fatal_memory_ops (const char*);

void fatal_max_nestedloop_level (const char*, const unsigned short, const unsigned short);

#endif
