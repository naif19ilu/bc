/* bc - brainfuck compiler
 * Jul 14, 2025
 * Lexer & Parser
 */
#include "lexpa.h"
#include "fatal.h"

#include <stdlib.h>

struct openLoopStack
{
	struct token *stack[OPENLOOP_STACK_MAX_CAP];
	unsigned short at;
};

static struct token *get_next_token (struct stream*);
static struct token *handle_accumulative (struct stream*, const char*, const unsigned short, const unsigned short);

static void handle_opening (struct openLoopStack*, struct stream*, const char*, const unsigned short, const unsigned short);
static void handle_closing (struct openLoopStack*, struct stream*, const char*, const unsigned short, const unsigned short);

void lexpa_lex_n_parse (const char *source, const size_t length, struct stream *stream)
{
	stream->length   = 0;
	stream->capacity = STREAM_GROWTH_FACTOR;
	stream->stream   = (struct token*) calloc(STREAM_GROWTH_FACTOR, sizeof(struct token));
	CHECK_POINTER(stream->stream, "reserving storage for tokens");

	unsigned short numline = 1, offline = 0;
	struct token *last = NULL;

	struct openLoopStack stack = {0};

	for (size_t i = 0; i < length; i++)
	{
		const char mnemonic = source[i];

		if (last && last->meta.mnemonic == mnemonic)
		{
			last->groupSize++;
			offline++;
			continue;
		}

		switch (mnemonic)
		{
			case '+':
			case '-':
			case '<':
			case '>':
			case ',':
			case '.':
			case '@': { last = handle_accumulative(stream, source + i, numline, offline); break; }
			case '[': { handle_opening(&stack, stream, source + i, numline, offline); break; }
			case ']': { handle_closing(&stack, stream, source + i, numline, offline); break; }
			case 10 : { numline++; offline = 0; continue; }
		}
		offline++;
	}

	bool leave = false;
	for (unsigned short i = 0; i < stack.at; i++)
	{
		struct token *e = stack.stack[i];
		fatal_source_fatal(e->meta.context, e->meta.numline, e->meta.offline, FATAL_SRC_UNMATCHED_OPEN, FATAL_ISNT_MULTIPLE);
		leave = true;
	}

	if (leave) { exit(EXIT_FAILURE); }
}

static struct token *get_next_token (struct stream *stream)
{
	if (stream->length == stream->capacity)
	{
		stream->capacity += STREAM_GROWTH_FACTOR;
		stream->stream   = (struct token*) realloc(stream->stream, sizeof(struct token) * stream->capacity);
		CHECK_POINTER(stream->stream, "reserving storage for tokens");
	}
	return &stream->stream[stream->length++];
}

static struct token *handle_accumulative (struct stream *stream, const char *context, const unsigned short numline, const unsigned short offline)
{
	struct token *t = get_next_token(stream);

	t->groupSize     = 1;
	t->meta.numline  = numline;
	t->meta.offline  = offline;
	t->meta.mnemonic = *context;
	t->meta.context  = (char*) context;

	return t;
}

static void handle_opening (struct openLoopStack *stack, struct stream *stream, const char *context, const unsigned short numline, const unsigned short offline)
{
	if (stack->at == OPENLOOP_STACK_MAX_CAP)
	{
		fatal_source_fatal(context, numline, offline, FATAL_SRC_MAX_NESTED_LEVEL, FATAL_ISNT_MULTIPLE);
	}

	struct token *t = get_next_token(stream);

	t->groupSize      = 1;
	t->parnerPosition = stream->length - 1;
	t->meta.numline   = numline;
	t->meta.offline   = offline;
	t->meta.mnemonic  = *context;
	t->meta.context   = (char*) context;

	stack->stack[stack->at++] = t;
}

static void handle_closing (struct openLoopStack *stack, struct stream *stream, const char *context, const unsigned short numline, const unsigned short offline)
{
	if (stack->at == 0)
	{
		fatal_source_fatal(context, numline, offline, FATAL_SRC_PREMATURE_OPENING, FATAL_ISNT_MULTIPLE);
	}

	struct token *close   = get_next_token(stream);
	struct token* open    = stack->stack[--stack->at];

	close->parnerPosition = open->parnerPosition;

	close->groupSize      = 1;
	close->meta.numline   = numline;
	close->meta.offline   = offline;
	close->meta.mnemonic  = *context;
	close->meta.context   = (char*) context;
}
