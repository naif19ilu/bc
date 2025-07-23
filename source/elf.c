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

#define LARGEST_INST_LENGTH     26

enum immxxsz
{
	IMM_08 = 1,
	IMM_16 = 2,
	IMM_32 = 4,
	IMM_64 = 8
};

struct jump
{
	unsigned long beforeJmp;
	unsigned long afterJmp;
	unsigned long offset;
};

struct objcode
{
	struct jump   *jmps;
	unsigned char *buffer;
	size_t        len;
	size_t        cap;
	unsigned long vrip;
	unsigned long jmp;
	enum immxxsz  immsz;
};

struct amd64inst
{
	unsigned char source[LARGEST_INST_LENGTH];
	const size_t  immOffset;
	const size_t  length;
};

static void init_elf_generator (struct objcode*, const unsigned long, const unsigned char);
static void dump_object_code (struct objcode*, const char*, const unsigned int, const unsigned char);

static void write_object_code (struct objcode*, const unsigned char*, const size_t);
static void insert_immxx_into_instruction (const unsigned long, size_t, const enum immxxsz, unsigned char*);

static void emmit_amd64_inc_dec (struct objcode*, const unsigned long, const char);
static void emmit_amd64_nxt_prv (struct objcode*, const unsigned long, const char);

static void emmit_amd64_out_inp (struct objcode*, const unsigned long, const char);
static void emmit_amd64_branches (struct objcode*, const char);

void elf_produce (const struct stream *stream, const char *filename, const unsigned int tapesz, const unsigned char cellsz)
{
	struct objcode obj = {0};
	init_elf_generator(&obj, stream->nonested, cellsz);

	for (size_t i = 0; i < stream->length; i++)
	{
		const struct token *token = &stream->stream[i];
		const char mnemonic = token->meta.mnemonic;

		switch (mnemonic)
		{
			case '+':
			case '-': emmit_amd64_inc_dec(&obj, token->groupSize, mnemonic); break;

			case '<':
			case '>': emmit_amd64_nxt_prv(&obj, token->groupSize, mnemonic); break;

			case ',':
			case '.': emmit_amd64_out_inp(&obj, token->groupSize, mnemonic); break;

			case '[':
			case ']': emmit_amd64_branches(&obj, mnemonic); break;
		}
	}

	dump_object_code(&obj, filename, tapesz, cellsz);
}

static void init_elf_generator (struct objcode *obj, const unsigned long nonested, const unsigned char cellsz)
{
	obj->cap    = BUFFER_GROWTH_FACTOR;
	obj->buffer = (unsigned char*) calloc(BUFFER_GROWTH_FACTOR, sizeof(*obj->buffer));

	CHECK_POINTER(obj->buffer, "reserving space for object code");
	obj->vrip = ENTRY_VIRTUAL_ADDRESS;

	const unsigned char intro[] =
	{
		/* lea r8, [rip + <add>]
		 * <add> will be determinated at 'dump_object_code' function once
		 * the compiler knows how big the .text section is
		 */
		0x4c, 0x8d, 0x05, 0x00, 0x00, 0x00, 0x00
	};

	write_object_code(obj, intro, sizeof(intro));
	obj->immsz = (enum immxxsz) cellsz;

	obj->jmps = (struct jump*) calloc(nonested, sizeof(struct jump));
	CHECK_POINTER(obj->jmps, "reserving space for branch-handling");
}

static void dump_object_code (struct objcode *obj, const char *filename, const unsigned int tapesz, const unsigned char cellsz)
{
	FILE *file = fopen(filename, "wb");
	if (file == NULL)
	{
		fatal_file_ops(filename);
	}

	const unsigned char outro[] =
	{
		/* mov rax, 60
		 * mov rdi, 0
		 * syscall
		 */
		0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00,
		0x48, 0xc7, 0xc7, 0x00, 0x00, 0x00, 0x00,
		0x0f, 0x05
	};
	write_object_code(obj, outro, sizeof(outro));

	/* We need to modify the address provided in 'init_elf_generator' function.
	 * The compiler can define the offset since it already knows how big objcode is.
	 *
	 * Offset is three since at fourth byte within buffer is where the offset is defined
	 * (see 'init_elf_generator.intro')
	 */
	insert_immxx_into_instruction(obj->len, 3, IMM_32, obj->buffer);

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
	 * 5. At offset 104: `p_memsz`  for text segment (object code length + tapesz) // TODO mul tapesz * cellsz
	 */
	insert_immxx_into_instruction(ENTRY_VIRTUAL_ADDRESS,      24 , IMM_64, elfprelude);
	insert_immxx_into_instruction(P_OFFSET_PROG_HEADER_1,     72 , IMM_64, elfprelude);
	insert_immxx_into_instruction(ENTRY_VIRTUAL_ADDRESS,      80 , IMM_64, elfprelude);
	insert_immxx_into_instruction(obj->len,                   96 , IMM_64, elfprelude);
	insert_immxx_into_instruction(obj->len + tapesz * cellsz, 104, IMM_64, elfprelude);

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

static void emmit_amd64_inc_dec (struct objcode *obj, const unsigned long imm, const char mnemonic)
{	
	static const struct amd64inst instructions[8] =
	{
		{
			/* subb [r8], imm8
			 * */
			.source =
			{
				0x41, 0x80, 0x28, 0x00
			},
			.immOffset = 3,
			.length    = 4,
		},
		{

			/* movw r9w, [r8]
			 * subw r9w, imm16
			 * movw [r8], r9w
			 */
			.source =
			{
				0x66, 0x45, 0x8b, 0x08,
				0x66, 0x41, 0x81, 0xe9, 0x00, 0x00,
				0x66, 0x45, 0x89, 0x08
			},
			.immOffset = 8,
			.length    = 14
		},
		{
			/* subd [r8], imm32
			 */
			.source =
			{
				0x41, 0x81, 0x28, 0x00, 0x00, 0x00, 0x00
			},
			.immOffset = 3,
			.length = 7
		},
		{
			/* movq rax, imm64
			 * subq [r8], rax
			 */
			.source =
			{
				0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x49, 0x29, 0x00
			},
			.immOffset = 2,
			.length = 13
		},
		{
			/* addb [r8], imm8
			 */
			.source =
			{
				0x41, 0x80, 0x00, 0x00,
			},
			.immOffset = 3,
			.length    = 4,
		},
		{
			/* movw r9w, [r8]
			 * addw r9w, imm16
			 * movw [r8], r9w
			 */
			.source =
			{
				0x66, 0x45, 0x8b, 0x08,
				0x66, 0x41, 0x81, 0xc1, 0x00, 0x00,
				0x66, 0x45, 0x89, 0x08
			},
			.immOffset = 8,
			.length    = 14
		},
		{
			/* addd [r8], imm32
			 */
			.source =
			{
				0x41, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00
			},
			.immOffset = 3,
			.length = 7
		},
		{
			/* movq rax, imm64
			 * addq [r8], rax
			 */
			.source =
			{
				0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x49, 0x01, 0x00
			},
			.immOffset = 2,
			.length = 13
		}
	};

	const unsigned int pick = ((mnemonic == '-') ? 0 : 4) + ((obj->immsz == 8) ? 3 : (obj->immsz >> 1));
	struct amd64inst instruction = instructions[pick];

	insert_immxx_into_instruction(imm, instruction.immOffset, obj->immsz, instruction.source);
	write_object_code(obj, instruction.source, instruction.length);
}

static void emmit_amd64_nxt_prv (struct objcode *obj, const unsigned long imm, const char mnemonic)
{
	static const struct amd64inst instructions[2] =
	{
		/* movq rax, imm64
		 * addq r8, rax
		 */
		{
			.source =
			{
				0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x49, 0x01, 0xc0
			},
			.immOffset = 2,
			.length = 13
		},
		/* movq rax, imm64
		 * subq r8, rax
		 */
		{
			.source =
			{
				0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x49, 0x29, 0xc0
			},
			.immOffset = 2,
			.length = 13
		}
	};

	const unsigned int pick = (mnemonic == '>') ? 0 : 1;
	struct amd64inst instruction = instructions[pick];

	insert_immxx_into_instruction(imm, instruction.immOffset, obj->immsz, instruction.source);
	write_object_code(obj, instruction.source, instruction.length);
}

static void emmit_amd64_out_inp (struct objcode *obj, const unsigned long times, const char mnemonic)
{
	static const struct amd64inst instructions[2] =
	{

		/* mov rax, 1
		 * mov rdi, 1
		 * mov rsi, r8
		 * mov rdx, 1
		 * syscall
		 */
		{
			.source =
			{
				0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,
				0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00,
				0x4c, 0x89, 0xc6,            
				0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,
				0x0f, 0x05,
			},
			.immOffset = 0,
			.length = LARGEST_INST_LENGTH
		},
		/* mov rax, 0
		 * mov rdi, 0
		 * mov rsi, r8
		 * mov rdx, 1
		 * syscall
		 */
		{
			.source =
			{
				0x48, 0xc7, 0xc0, 0x00, 0x00, 0x00, 0x00,
				0x48, 0xc7, 0xc7, 0x00, 0x00, 0x00, 0x00,
				0x4c, 0x89, 0xc6,            
				0x48, 0xc7, 0xc2, 0x01, 0x00, 0x00, 0x00,
				0x0f, 0x05,
			},
			.immOffset = 0,
			.length = LARGEST_INST_LENGTH
		}
	};

	const unsigned int pick = (mnemonic == '.') ? 0 : 1;
	struct amd64inst instruction = instructions[pick];

	for (unsigned long i = 0; i < times; i++)
	{
		write_object_code(obj, instruction.source, instruction.length);
	}
}

static void emmit_amd64_branches (struct objcode *obj, const char mnemonic)
{
	if (mnemonic == ']')
	{
		struct amd64inst str8jmp =
		{
			/* movq rax, <address>
			 * jmp rax
			 */
			.source =
			{
				0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xff, 0xe0
			},
			.immOffset = 2,
			.length = 12
		};

		struct jump *last = &obj->jmps[--obj->jmp];
		const unsigned int relative = (obj->vrip + str8jmp.length) - last->afterJmp;

		/* patches:
		 * 1. relative address when '[' was defined
		 * 2. absolute address where to jump everytime ']' is found
		 */
		insert_immxx_into_instruction(relative, last->offset, IMM_32, obj->buffer);
		insert_immxx_into_instruction(last->beforeJmp, str8jmp.immOffset, IMM_32, str8jmp.source);

		write_object_code(obj, str8jmp.source, str8jmp.length);
		return;
	}

	static const struct amd64inst instructions[4] =
	{
		{
			/* movb al, [r8]
			 * cmpb al, 0
			 * je   <address>
			 */
			.source =
			{
				0x41, 0x8a, 0x00,
				0x3c, 0x00,
				0x0f, 0x84, 0x00, 0x00, 0x00, 0x00
			},
			.immOffset = 7,
			.length = 11
		},
		{
			/* movw ax, [r8]
			 * cmpw ax, 0
			 * je   <address>
			 */
			.source =
			{
				0x66, 0x41, 0x8b, 0x00,
				0x66, 0x3d, 0x00, 0x00,
				0x0f, 0x84, 0x00, 0x00, 0x00, 0x00
			},
			.immOffset = 10,
			.length = 14
		},
		{
			/* movl eax, [r8]
			 * cmpl eax, 0
			 * je   <address>
			 */
			.source =
			{
				0x41, 0x8b, 0x00,
				0x3d, 0x00, 0x00, 0x00, 0x00,
				0x0f, 0x84, 0x00, 0x00, 0x00, 0x00
			},
			.immOffset = 10,
			.length = 14
		},
		{
			/* movq rax, [r8]
			 * cmpq rax, 0
			 * je   <address>
			 */
			.source =
			{
				0x49, 0x8b, 0x00,
				0x48, 0x83, 0xf8, 0x00,
				0x0f, 0x84, 0x00, 0x00, 0x00, 0x00
			},
			.immOffset = 9,
			.length = 13
		},
	};

	const unsigned int pick = ((obj->immsz == 8) ? 3 : (obj->immsz >> 1));
	struct amd64inst instruction =  instructions[pick];

	struct jump *jmp = &obj->jmps[obj->jmp++];
	jmp->offset      = obj->len + instruction.immOffset;
	/* address before writing the instruction.source, this is needed since this is the absolute
	 * address where ] will jump
	 */
	jmp->beforeJmp   = obj->vrip;

	write_object_code(obj, instruction.source, instruction.length);
	/* address after writing the instruction.source, this is needed since it's used to calculate
	 * the relative address for '[' jumps
	 */
	jmp->afterJmp = obj->vrip;
}
