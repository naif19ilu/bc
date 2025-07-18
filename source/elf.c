/* bc - brainfuck compiler
 * Jul 16, 2025
 * ELF file generator
 */
#include "elf.h"
#include "fatal.h"

#define BUFFER_LENGTH     2048

#define IMM_8_BITS        1
#define IMM_16_BITS       2
#define IMM_32_BITS       4
#define IMM_64_BITS       8

struct elfgen
{
	FILE           *file;
	struct         { char buffer[BUFFER_LENGTH]; size_t at; } buffer;
	char           *filename;
	unsigned long  offset;
	unsigned int   bsstarts;
	unsigned int   npages;
	unsigned char  cellwidth;
	enum arch      arch;
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

static void emmit_header (struct elfgen*);
static void write_code (struct stream*, struct elfgen*);

static void emmit_amd64_inc_dec (const unsigned long, struct elfgen*, const char);
static void emmit_amd64_nxt_prv (const unsigned long, struct elfgen*, const char);

void elf_produce_elf (struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned char cellSize, const enum arch arch)
{
	struct elfgen elfg =
	{
		.file      = fopen(filename, "wb"),
		.filename  = (char*) filename,
		.offset    = 0x0401000,
		.npages    = ((unsigned int) stream->length / 4096) + 1,
		.cellwidth = cellSize,
		.arch      = arch
	};

	elfg.bsstarts  = 0x401000 + elfg.npages * 4096;
	if (!elfg.file) { fatal_file_ops(filename); }

	emmit_header(&elfg);
	write_code(stream, &elfg);

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

static void emmit_header (struct elfgen *elfg)
{
	if (elfg->arch == ARCH_AMD64)
	{
		unsigned char amd64[8] = { 0x4c, 0x8d, 0x84, 0x24 };
		get_little_endian((unsigned long) elfg->bsstarts, 4, IMM_32_BITS, amd64);
		write_instruction(elfg, amd64, 8);
		return;
	}
}

static void write_code (struct stream *stream, struct elfgen *elfg)
{
	for (size_t i = 0; i < stream->length; i++)
	{
		struct token *token = &stream->stream[i];
		switch (token->meta.mnemonic)
		{
			case '+':
			case '-': emmit_amd64_inc_dec(token->groupSize, elfg, token->meta.mnemonic); break;

			case '>':
			case '<': emmit_amd64_nxt_prv(token->groupSize, elfg, token->meta.mnemonic); break;
		}
	}
}

static void emmit_amd64_inc_dec (const unsigned long times, struct elfgen *elfg, const char mnemonic)
{
	unsigned char code[][8] = {
		{ 0x41, 0x80, 0x28,            (unsigned char) times  },
		{ 0x66, 0x41, 0x81, 0x28,      0x00, 0x00             },
		{ 0x41, 0x81, 0x28,            0x00, 0x00, 0x00, 0x00 },
		{ 0x49, 0x81, 0x28,            0x00, 0x00, 0x00, 0x00 },

		{ 0x41, 0x80, 0x00,            (unsigned char) times  },
		{ 0x66, 0x41, 0x81, 0x00,      0x00, 0x00             },
		{ 0x41, 0x81, 0x00,            0x00, 0x00, 0x00, 0x00 },
		{ 0x49, 0x81, 0x00,            0x00, 0x00, 0x00, 0x00 }
	};

	const unsigned int write = ((mnemonic == '-') ? 0 : 4) + ((elfg->cellwidth == 8) ? 3 : (elfg->cellwidth >> 1));

	if (elfg->cellwidth != 1)
	{
		get_little_endian(
			times,
			(elfg->cellwidth == 2) ? 4 : 3,
			elfg->cellwidth,
			code[write]
		);
	}

	switch (elfg->cellwidth)
	{
		case 1: write_instruction(elfg, code[write], 4); break;
		case 2: write_instruction(elfg, code[write], 6); break;
		case 4:
		case 8: write_instruction(elfg, code[write], 7); break;
	}
}

static void emmit_amd64_nxt_prv (const unsigned long times, struct elfgen *elfg, const char mnemonic)
{
	unsigned char code[][7] = {
		{0x49, 0x81, 0xe8, 0x00, 0x00, 0x00, 0x00},
		{0x49, 0x81, 0xc0, 0x00, 0x00, 0x00, 0x00},
	};

	const unsigned int write = (mnemonic == '>');
	get_little_endian(times, 3, IMM_32_BITS, code[write]);
	write_instruction(elfg, code[write], 7);
}
