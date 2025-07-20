/* bc - brainfuck compiler
 * Jul 16, 2025
 * ELF file generator
 */
#include "elf.h"
#include "fatal.h"

#include <stdlib.h>

#define BUFFER_GROWTH_FACTOR    2048
#define PAGE_SIZE               4096

#define ENTRY_VIRTUAL_ADDRESS   0x101078
#define P_OFFSET_PROG_HEADER_1  0x78

#define ELF_HEADER_LENGTH       64
#define PROGRAM_HEADER_LENGTH   56
#define ELF_PRELUDE_LENGTH      (ELF_HEADER_LENGTH + PROGRAM_HEADER_LENGTH)

enum immxxsz
{
	IMM_08 = 1,
	IMM_16 = 2,
	IMM_32 = 4,
	IMM_64 = 8
};

struct objcode
{
	unsigned char *buffer;
	size_t        len;
	size_t        cap;
	unsigned long vrip;
};

struct elfgen
{
	struct objcode obj;
};

static void init_elf_generator (struct elfgen*);
static void dump_object_code (struct objcode*, const char*, const unsigned int);

static void write_object_code (struct objcode*, const unsigned char*, const size_t);
static void insert_immxx_into_instruction (const unsigned long, size_t, const enum immxxsz, unsigned char*);

void elf_produce (const struct stream *stream, const char *filename, const unsigned int tapesz, const unsigned char cellsz)
{
	struct elfgen elfg;
	init_elf_generator(&elfg);

	dump_object_code(&elfg.obj, filename, tapesz);
}

static void init_elf_generator (struct elfgen *elfg)
{
	elfg->obj.cap    = BUFFER_GROWTH_FACTOR;
	elfg->obj.buffer = (unsigned char*) calloc(BUFFER_GROWTH_FACTOR, sizeof(*elfg->obj.buffer));
	elfg->obj.vrip   = ENTRY_VIRTUAL_ADDRESS;
}

static void dump_object_code (struct objcode *obj, const char *filename, const unsigned int tapesz)
{
	FILE *file = fopen(filename, "wb");
	if (file == NULL)
	{
		fatal_file_ops(filename);
	}

	static const unsigned char outro[] =
	{
		/* mov rax, 60
		 * mov rdi, 0
		 * syscall
		 */
		0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00,
		0x48, 0xc7, 0xc7, 0x3c, 0x00, 0x00, 0x00,
		0x0f, 0x05
	};
	write_object_code(obj, outro, sizeof(outro));

	unsigned char elfprelude[] =
	{
		0x7f, 0x45, 0x4c, 0x46,
		0x02,
		0x01,
		0x01,
		0x00,
		0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00,
		0x3e, 0x00,
		0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x40, 0x00,
		0x38, 0x00,
		0x01, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,

		0x01, 0x00, 0x00, 0x00,
		0x07, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	/* patches:
	 * 1. At offset 24 : `e_entry`
	 * 2. At offset 72 : `p_offset` for text segment
	 * 3. At offset 80 : `p_vaddr`  for text segment
	 * 4. At offset 96 : `p_filesz` for text segment (object code length)
	 * 5. At offset 104: `p_memsz`  for text segment (object code length + tapesz)
	 */
	insert_immxx_into_instruction(ENTRY_VIRTUAL_ADDRESS,  24 , IMM_64, elfprelude);
	insert_immxx_into_instruction(P_OFFSET_PROG_HEADER_1, 72 , IMM_64, elfprelude);
	insert_immxx_into_instruction(ENTRY_VIRTUAL_ADDRESS,  80 , IMM_64, elfprelude);
	insert_immxx_into_instruction(obj->len,               96 , IMM_64, elfprelude);
	insert_immxx_into_instruction(obj->len + tapesz,      104, IMM_64, elfprelude);

	if (fwrite(elfprelude, 1, sizeof(elfprelude), file) != ELF_PRELUDE_LENGTH)
	{
		fatal_file_ops(filename);
	}
	if (fwrite(obj->buffer, 1, obj->len, file) != obj->len) { fatal_file_ops(filename); }
	if (fclose(file))                                       { fatal_file_ops(filename); }
}

static void write_object_code (struct objcode *obj, const unsigned char *instruction, const size_t length)
{
	if ((obj->len + length) >= obj->cap)
	{
		obj->cap += BUFFER_GROWTH_FACTOR;
		obj->buffer = (unsigned char*) realloc(obj->buffer, sizeof(*obj->buffer) * obj->cap);
		CHECK_POINTER(obj->buffer, "reserving space for object code");
	}

	for (size_t i = 0; i < length; i++)
	{
		obj->buffer[obj->len++] = instruction[i];
		obj->vrip++;
	}
}

static void insert_immxx_into_instruction (const unsigned long imm, size_t offset, const enum immxxsz sz, unsigned char *buff)
{
	for (register unsigned char i = 0; i < sz; i++)
	{
		buff[offset++] = ((imm >> (i * 8)) & 0xff);
	}
}
