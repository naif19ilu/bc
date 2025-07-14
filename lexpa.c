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

void lexpa_run (const char *source, const size_t length, struct stream *stream, const bool safeMode)
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
			case '.': { last = handle_accumulative(stream, source + i, numline, offline); break; }
			case '[': { handle_opening(&stack, stream, source + i, numline, offline); break; }
			case ']':
			case 10 : { numline++; offline = 0; continue; }
		}

		offline++;
	}

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
		fatal_max_nestedloop_level(context, numline, offline);
	}

	struct token *t = get_next_token(stream);

	t->groupSize      = 1;
	t->parnerPosition = -1;
	t->meta.numline   = numline;
	t->meta.offline   = offline;
	t->meta.mnemonic  = *context;
	t->meta.context   = (char*) context;

	stack->stack[stack->at++] = t;
}
