/* bc - brainfuck compiler
 * Jul 16, 2025
 * ELF file generator
 */
#ifndef BC_ELF_H
#define BC_ELF_H
#include "bc.h"

void elf_produce_elf (struct stream*, const char*, const unsigned int, const unsigned char, const enum arch);

#endif
