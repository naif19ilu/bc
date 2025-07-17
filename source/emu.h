/* bc - brainfuck compiler
 * Jul 14, 2025
 * Memory (tape) emulator
 */
#ifndef BC_EMU_H
#define BC_EMU_H
#include "bc.h"

void emu_emulate (const struct stream*, const unsigned int, const unsigned char, const bool, const unsigned int, const unsigned int, const unsigned int);

#endif
