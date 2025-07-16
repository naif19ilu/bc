/* bc - brainfuck compiler
 * Jul 15, 2025 Happy birthday!!
 * Assembly generator (-S)
 */
#ifndef BC_ASM_H
#define BC_ASM_H
#include "bc.h"

void asm_gen_asm (const struct stream*, const char*, const unsigned int, const unsigned char);

#endif
