/* bc - brainfuck compiler
 * Jul 14, 2025
 * Fatals handler
 */
#define _GNU_SOURCE

#include "fatal.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* 'strerrordesc_np' function returns in fact a string
 * but gcc does not recognize it so better redefine the
 * defintion
 */
const char* strerrordesc_np(int);

void fatal_file_ops (const char *filename)
{
	const char *const fmt =
	"bc:\x1b[31mfatal:\x1b[0m cannot continue due to file issues\n"
	"  reason: %s\n"
	"  file  : %s\n"
	"  aborting now!\n";
	fprintf(stderr, fmt, strerrordesc_np(errno), filename);
	exit(EXIT_FAILURE);
}

void fatal_memory_ops (const char *desc)
{
	const char *const fmt =
	"bc:\x1b[31mfatal:\x1b[0m cannot continue due to memory managment issues\n"
	"  reason: %s\n"
	"  while : %s\n"
	"  aborting now!\n";
	fprintf(stderr, fmt, strerrordesc_np(errno), desc);
	exit(EXIT_FAILURE);
}

