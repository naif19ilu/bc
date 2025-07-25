/* bc - brainfuck compiler
 * Jul 14, 2025
 * Fatals handler
 */
#define _GNU_SOURCE

#include "fatal.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* 'strerrordesc_np' function returns in fact a string
 * but gcc does not recognize it so better redefine the
 * defintion
 */
const char* strerrordesc_np(int);

static unsigned short get_proper_context (const char*);

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
	"bc:\x1b[31mfatal:\x1b[0m cannot continue due to memory management issues\n"
	"  reason: %s\n"
	"  while : %s\n"
	"  aborting now!\n";
	fprintf(stderr, fmt, strerrordesc_np(errno), desc);
	exit(EXIT_FAILURE);
}

void fatal_source_fatal (const char *context, const unsigned short numline, const unsigned short offline, const enum FatalSourceKind kind, const enum FatalIsMultiple ismul)
{
	static const char *const reasons[] =
	{
		"bc:\x1b[31mfatal:\x1b[0m max nested loop level reached! What are you even progrmming at this point?\n",
		"bc:\x1b[31mfatal:\x1b[0m premature opening, defining a ']' without previous '['\n",
		"bc:\x1b[31mfatal:\x1b[0m undefined closing, a '[' was defined without final ']'\n",
		"bc:\x1b[31mfatal:\x1b[0msafe-mode: memory overflow\n",
		"bc:\x1b[31mfatal:\x1b[0msafe-mode: memory underflow\n",
		"bc:\x1b[31mfatal:\x1b[0msafe-mode: cell overflow\n",
		"bc:\x1b[31mfatal:\x1b[0msafe-mode: cell underflow\n",
	};

	fprintf(stderr, "%s", reasons[kind]);
	const unsigned short show = get_proper_context(context + 1);
	fprintf(stderr, "  %-5d \x1b[5m%c\x1b[0m%.*s\n", numline, *context, show, context + 1);
	fprintf(stderr, "        ~ offset: %d\n", offline);
	
	if (ismul == FATAL_IS_MULTIPLE) { fputc(10, stderr); }
	else { exit(EXIT_FAILURE); }
}

static unsigned short get_proper_context (const char *cx)
{
	unsigned short i = 0;
	for (; cx[i] != ' ' && i <= 10 && cx[i] != '\n'; i++)
		 ;
	return i;
}

void fatal_nonfatal_warn (const enum FatalWarningKind kind, ...)
{
	static const char *const reasons[] =
	{
		"invalid argument for -C (%d), it can only be 1,2,4 or 8; setting to default (%d)\n\n",
		"invalid argument for -T (%d), it must be greater than 30000; setting to default (%d)\n\n",
		"invalid values for -O (%d) and -T (%d), -T must be greater than -O; setting both to default\n\n",
		"invalid values for -d (%d) and -T (%d) (or maybe -T < -d + -O which is not possible), -T must be greater than -d; setting both to default\n\n",
		"invalid value for -g (%d), cannot be zero; setting to default (%d)\n\n"
	};

	va_list args;
	va_start(args, kind);
	fprintf(stdout, "bc:\x1b[33mwarning:\x1b[0m hey! non fatal but be aware of it!\n");
	vfprintf(stdout, reasons[kind], args);
	va_end(args);
}
