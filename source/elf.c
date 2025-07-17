/* bc - brainfuck compiler
 * Jul 16, 2025
 * ELF file generator
 */
#include "elf.h"
#include "fatal.h"

#define INST_MAX_LENGTH   8
#define BUFFER_LENGTH     2048

struct instruction
{
	unsigned char code[INST_MAX_LENGTH];
	unsigned char length;
};

struct elfgen
{
	FILE           *file;
	struct         { char buffer[BUFFER_LENGTH]; size_t at; } buffer;
	char           *filename;
	unsigned long  offset;
	unsigned short npages;
	unsigned char  cellwidth;
};

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

static void write_instruction (struct elfgen *elfg, const struct instruction inst)
{
	check_buffer_capacity(elfg, inst.length);
	for (unsigned char i = 0; i < inst.length; i++)
	{
		elfg->buffer.buffer[elfg->buffer.at++] = inst.code[i];
		elfg->offset++;
	}
}

static void emmit_header (struct stream *stream, struct elfgen *elfg)
{
	static const unsigned char const amd64[] =
	{
		0x55,
		0x48, 0x89, 0xe5,
	};
}

void elf_produce_elf (struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned char cellSize, const enum arch arch)
{
	struct elfgen elfg =
	{
		.file      = fopen(filename, "wb"),
		.filename  = filename,
		.offset    = 0x0401000,
		.npages    = 1,
		.cellwidth = cellSize,
	};

	if (!elfg.file) { fatal_file_ops(filename); }

	emmit_header(stream, &elfg);
	if (fclose(elfg.file)) { fatal_file_ops(filename); }
}

