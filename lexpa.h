/* bc - brainfuck compiler
 * Jul 14, 2025
 * Lexer & Parser
 */
#ifndef BC_LEXPA_H
#define BC_LEXPA_H
#include "bc.h"

void lexpa_lex_n_parse (const char*, const size_t, struct stream*);

#endif
