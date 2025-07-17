/* bc - brainfuck compiler
 * Jul 16, 2025
 * ELF file generator
 */
#include "elf.h"
#include "fatal.h"

#define BUFFER_LENGTH     2048

struct elfgen
{
	FILE           *file;
	struct         { char buffer[BUFFER_LENGTH]; size_t at; } buffer;
	char           *filename;
	unsigned long  offset;
	unsigned int   bsstarts;
	unsigned short npages;
	unsigned char  cellwidth;
};

inline static void get_little_endian (const unsigned long imm, const unsigned short offset, const unsigned short immsz, unsigned char *inst)
{
	for (unsigned short i = offset, j = 0; i < offset + immsz; i++)
	{
		inst[i] = (unsigned char) ((imm >> (8 * j++)) & 0xff);
	}
}

static void check_buffer_capacity (struct elfgen*, const unsigned char);
static void write_instruction (struct elfgen*, const unsigned char*, const unsigned char);

static void emmit_header (struct stream*, struct elfgen*);

void elf_produce_elf (struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned char cellSize, const enum arch arch)
{
	struct elfgen elfg =
	{
		.file      = fopen(filename, "wb"),
		.filename  = (char*) filename,
		.offset    = 0x0401000,
		.npages    = (unsigned short) (stream->length / 4096) + 1,
		.bsstarts  = 0x401000 + elfg.npages,
		.cellwidth = cellSize,
	};

	if (!elfg.file) { fatal_file_ops(filename); }

	emmit_header(stream, &elfg);
	if (fwrite(elfg.buffer.buffer, 1, elfg.buffer.at, elfg.file) != elfg.buffer.at)
	{
		fatal_file_ops(elfg.filename);
	}

	if (fclose(elfg.file)) { fatal_file_ops(filename); }
}

static void check_buffer_capacity (struct elfgen *elfg, const unsigned char instsz)
{
	if ((elfg->buffer.at + instsz) >= BUFFER_LENGTH)
	{
		if (fwrite(elfg->buffer.buffer, 1, elfg->buffer.at, elfg->file) != elfg->buffer.at)
		{
			fatal_file_ops(elfg->filename);
		}
		elfg->buffer.at = 0;
	}
}

static void write_instruction (struct elfgen *elfg, const unsigned char *inst, const unsigned char length)
{
	check_buffer_capacity(elfg, length);
	for (unsigned char i = 0; i < length; i++)
	{
		elfg->buffer.buffer[elfg->buffer.at++] = inst[i];
		elfg->offset++;
	}
}

static void emmit_header (struct stream *stream, struct elfgen *elfg)
{
	unsigned char amd64[32] = { 0x4c, 0x8d, 0x84 };
	get_little_endian((unsigned long) elfg->bsstarts, 3, 4, amd64);

	write_instruction(elfg, amd64, 7);
}

