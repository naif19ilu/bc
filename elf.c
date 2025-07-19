/* bc - brainfuck compiler
 * Jul 16, 2025
 * ELF file generator
 */
#include "elf.h"
#include "fatal.h"

#define IMM_8_BITS     1
#define IMM_16_BITS    2
#define IMM_32_BITS    4
#define IMM_64_BITS    8
#define PAGE_SIZE_KB   4096

#include <stdlib.h>

struct elfgen
{
	struct         { unsigned char *buffer; size_t at, cap; } buffer;
	struct         { unsigned long *at, nth; } ujmps;
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

static void open_and_write_elf (struct elfgen*, const char*, const unsigned int);
static void write_instruction (struct elfgen*, const unsigned char*, const unsigned char);

static void emmit_header (struct elfgen*);
static void resolve_stream (struct stream*, struct elfgen*);

static void emmit_amd64_inc_dec (const unsigned long, struct elfgen*, const char);
static void emmit_amd64_nxt_prv (const unsigned long, struct elfgen*, const char);

static void emmit_amd64_out_inp (const unsigned long, struct elfgen*, const char);
static void emmit_amd64_lhs_rhs (struct elfgen*, struct token*, const char);

void elf_produce_elf (struct stream *stream, const char *filename, const unsigned int tapeSize, const unsigned char cellSize, const enum arch arch)
{
	struct elfgen elfg =
	{
		.buffer.cap     = PAGE_SIZE_KB,
		.buffer.buffer  = (unsigned char*) calloc(PAGE_SIZE_KB, sizeof(unsigned char)),
		.ujmps.at       = (unsigned long*) calloc(stream->nonested, sizeof(unsigned long)),
		.offset         = 0x401000,
		.npages         = ((unsigned int) stream->length / PAGE_SIZE_KB) + 1,			// bad calculated XXX
		.cellwidth      = cellSize,
		.arch           = arch
	};

	/* default position to place the first instruction is at 0x401000 (text section somewhat)
	 * for placing the memory (in this case) we will pick 0x401000 + number of pages used
	 * for writing the code, each page is 4 kB
	 */
	elfg.bsstarts = 0x401000 + elfg.npages * PAGE_SIZE_KB;

	CHECK_POINTER(elfg.buffer.buffer, "reserving space for bytecode");
	CHECK_POINTER(elfg.ujmps.at, "unresolved jumps, internal stuff my man");

	emmit_header(&elfg);
	resolve_stream(stream, &elfg);
	open_and_write_elf(&elfg, filename, tapeSize);
}

static void open_and_write_elf (struct elfgen *elfg, const char *filename, const unsigned int tapeSize)
{
	FILE *file = fopen(filename, "wb");
	if (!file)
	{
		fatal_file_ops(filename);
	}

	const unsigned long written = 64 + 56;
	unsigned char elf[64 + 56] =
	{
		/* magic number         */	0x7f, 0x45, 0x4c, 0x46,
		/* 64 or 32 bit         */	0x02,
		/* endianess            */	0x01,
		/* ELF version          */	0x01,
		/* system ABI           */	0x00,
		/* ABI stuff            */	0x00,
		/* Padding              */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* its a exec file      */	0x20, 0x00,
		/* target arch          */	0x3e, 0x00,
		/* ELF version          */	0x01, 0x00, 0x00, 0x00,
		/* Entry point          */	0x00, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* P. Header table at   */	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* S. Header table at   */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* e_flags              */	0x00, 0x00, 0x00, 0x00,
		/* ELF header size      */	0x40, 0x00,
		/* Program header size  */	0x38, 0x00,
		/* No. Prog. headers    */	0x01, 0x00,
		/* size of hdr. section */	0x00, 0x00,
		/* e_shnum              */	0x00, 0x00,
		/* e_shstrndx           */	0x00, 0x00,

		/* type of segment      */	0x01, 0x00, 0x00, 0x00,
		/* permissions          */	0x05, 0x00, 0x00, 0x00,
		/* segment's offset     */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* virtual address      */	0x00, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* physical address     */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* bytes within file    */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* bytes within v. addr */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		/* aligment             */	0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	get_little_endian(elfg->buffer.at, 96, IMM_64_BITS, elf);
	get_little_endian(elfg->buffer.at + tapeSize, 104, IMM_64_BITS, elf);

	fwrite(elf, 1, written, file);

	const unsigned char zeroes[] = {0};
	for (unsigned long i = written; i < PAGE_SIZE_KB; i++)
	{
		fwrite(zeroes, 1, 1, file);
	}

	if (fwrite(elfg->buffer.buffer, 1, elfg->buffer.at, file) != elfg->buffer.at)
	{
		fatal_file_ops(filename);
	}
	if (fclose(file)) { fatal_file_ops(filename); }
}

static void write_instruction (struct elfgen *elfg, const unsigned char *inst, const unsigned char length)
{
	if ((elfg->buffer.at + length) >= elfg->buffer.cap)
	{
		elfg->buffer.cap += PAGE_SIZE_KB;
		elfg->buffer.buffer = (unsigned char*) realloc(elfg->buffer.buffer, elfg->buffer.cap);
		CHECK_POINTER(elfg->buffer.buffer, "reserving space for bytecode");
	}

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

static void resolve_stream (struct stream *stream, struct elfgen *elfg)
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

			case '.':
			case ',': emmit_amd64_out_inp(token->groupSize, elfg, token->meta.mnemonic); break;

			case ']': emmit_amd64_lhs_rhs(elfg, token, token->meta.mnemonic); break;
			case '[': emmit_amd64_lhs_rhs(elfg, &stream->stream[token->parnerPosition], token->meta.mnemonic); break;
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

static void emmit_amd64_out_inp (const unsigned long times, struct elfgen *elfg, const char mnemonic)
{
	unsigned char code[] =
	{
		/*                ~~~~ this bytes indicates whether it is a write or read syscall (sysb) */
		0x48, 0xc7, 0xc0, 0x00, 0x00, 0x00, 0x00,
		0x48, 0xc7, 0xc7, 0x00, 0x00, 0x00, 0x00,
		0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,
		0x4c, 0x89, 0xc6,
		0x0f, 0x05
	};

	unsigned const char sysb = (mnemonic == '.');
	code[ 3] = sysb;
	code[10] = sysb;

	for (register unsigned long i = 0; i < times; i++)
	{
		write_instruction(elfg, code, 26);
	}
}

static void emmit_amd64_lhs_rhs (struct elfgen *elfg, struct token *close, const char mnemonic)
{
	const bool usg64 = (elfg->cellwidth == 8);

	if (mnemonic == ']')
	{
		/* overwriting the address that was left behind when implementing [ token */
		const unsigned long offset = elfg->ujmps.at[--elfg->ujmps.nth];
		get_little_endian(elfg->offset, offset, IMM_32_BITS, elfg->buffer.buffer);

		/* movq rax, <address>
		 * jmp  rax
		 */
		unsigned char code[] =
		{
			0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0xff, 0xe0
		};
		get_little_endian(close->jmp, 2, IMM_64_BITS, code);
		write_instruction(elfg, code, 12);
		return;
	}

	static const unsigned char code[][15] =
	{
		/* movq rax, [r8]
		 * cmpq rax, 0
		 * je   <address_rel32>
		 */
		{
			0x49, 0x8b, 0x00,
			0x48, 0x83, 0xf8, 0x00,
			0x0f, 0x84, 0x00, 0x00, 0x00, 0x00
		},

		/* movxz  eax, [r8]
		 * cmpd   eax, 0
		 * je     <address_rel32>
		 */
		{
			0x41, 0x0f, 0xb6, 0x00,
			0x3d, 0x00, 0x00, 0x00, 0x00,
			0x0f, 0x84, 0x00, 0x00, 0x00, 0x00
		}
	};

	/* for the ']' token we define the jump address as absolute
	 * since the `jmp` allows absolute jumps :)
	 */
	close->jmp = elfg->offset;

	const unsigned long overwriteAt = elfg->buffer.at + (usg64 ? 9 : 11);
	elfg->ujmps.at[elfg->ujmps.nth++] = overwriteAt;

	/* the code will be written but since we do not know the address where to jump
	 * yet it will be set to zero and whenever ] is found, we will overwrite the
	 * address to the actual number
	 */
	write_instruction(elfg, code[elfg->cellwidth != 8], (usg64 ? 13 : 15));
}

