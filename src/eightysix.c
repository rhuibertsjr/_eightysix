#include "eightysix.h"

#include <stdio.h>
#include <string.h>

uint8_t buffer[ARENA_DEFAULT_COMMIT_SIZE];

//- rhjr: string
String8
str8(uint8_t *cstring, size_t length)
{
  String8 result = { cstring, length };
	return result; 
}

String8
str8_range(uint8_t *first_char, uint8_t *last_char)
{
  String8 result = { first_char, (uint64_t)(last_char - first_char) };
	return result; 
}

String8
str8_cstring(uint8_t *cstring)
{
	uint8_t *ptr = cstring;
  for(; *ptr != 0; ptr += 1);
	return str8_range(cstring, ptr);
}

void
str8_list_push (Arena *arena, String8List *list, String8 string)
{
	String8Node node = { string, 0 };

	String8Node *ptr =
    (String8Node*) arena_alloc(arena, sizeof(String8Node));

	if (list->first == NULL)
  {
	  list->first = list->last = ptr;
  }
	else
  {
		list->last->next = ptr;
		list->last = ptr;
	}
		
	memcpy(ptr, &node, sizeof(String8Node));

	list->total_length += string.length;
	list->node_count += 1;
}

String8
str8_list_join(Arena *arena, String8List *list)
{
  // rhjr: per node_count '\r\n' and the last '\0'.
	uint8_t length = list->total_length + (list->node_count * 2) + 1;

	// rhjr: Total length + joint character '\r\n' (Windows).
	uint8_t *str =
		(uint8_t*) arena_alloc(arena, length);
	uint8_t *ptr = str; // rhjr: used to iterate. 

  //- rhjr: string construction
	for(String8Node *current = list->first; current != NULL; current =
    current->next)
	{
		memcpy(ptr, current->string.content, current->string.length);
		ptr += current->string.length;

		*ptr++ = (uint8_t) '\r';
		*ptr++ = (uint8_t) '\n';
	}

	*ptr = (uint8_t) '\0';

	return str8(str, length);
}


//- rhjr: arena
void
arena_init(Arena *arena, void *buffer, size_t size)
{
	arena->buffer = (uint8_t*) buffer;
	arena->size   = size;
	arena->offset = 0;
}

void *
arena_alloc(Arena *arena, size_t size)
{
	// TODO(rhjr): Fix memory alignment
	if (arena->offset + size <= arena->size)
  {
		void *ptr = (uint8_t*) arena->buffer + arena->offset;
		arena->offset += size;

		memset(ptr, 0, size);
		return ptr;
	}

	return NULL;
}

void
arena_free(Arena *arena)
{
	arena->offset = 0;
}


//- rhjr: io
void *
io_read_file(Arena *dst, const char* src)
{
	FILE    *file        = NULL;
	uint32_t file_length = 0;
	void    *result      = NULL;

	file = fopen(src, "rb");

	if (file == NULL)
  {
	  IO_RETURN_DEFER(NULL);
  }

	fseek(file, 0, SEEK_END);
  file_length = ftell(file);
	fseek(file, 0, SEEK_SET);

	result = (void*) arena_alloc(dst, file_length);
	
	uint32_t processed_chars =
		fread(result, sizeof(uint8_t), file_length, file);

	if (processed_chars == 0)
  {
	  IO_RETURN_DEFER(NULL);
  }

  defer:
    fclose(file);
	return result;
}	

uint32_t
io_write_file (const char *dst, String8 *src)
{
	FILE    *file   = NULL; 
	uint32_t result = 0;

	file = fopen(dst, "wb");

	if (file == NULL)
  {
	  IO_RETURN_DEFER(-1);
  }

  result = fprintf(file, "%s", src->content);

  if (result == 0) 
  {
	  IO_RETURN_DEFER(-1);
  }

  defer:
    fclose(file);
	return result;
}


//- rhjr: instruction decoding
void inst_decoding(Arena *arena, String8List *result, Inst *instruction)
{
  const uint32_t buffer_size = 10;
  String8 output = {0};

  // rhjr: assembly mnemonics
  char *opcode_mnemonic;
  char *operand1_mnemonic = "";
  char *operand2_mnemonic = "";

  char *register_lookup_table[] = {
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
  char *register_lookup_table_wide[] = {
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};

  //- rhjr: decoding
  uint8_t opcode = instruction->opcode >> 2;
  uint8_t dest, width, mod;
  uint8_t reg1, reg2;

  switch(opcode)
  {
    case MOV:
    { 
      opcode_mnemonic = "mov";
      dest  = instruction->opcode & 0b00000010;
      width = instruction->opcode & 0b00000001;

      reg1 = (instruction->operands & 0b00111000) >> 3;
      reg2 = instruction->operands & 0b00000111;

      mod = (instruction->operands & 0b11000000) >> 6;

      // rhjr: operand1 -> destination, operand2 -> source.
      operand1_mnemonic = (width == 0) ? register_lookup_table[reg2] :
                          register_lookup_table_wide[reg2];

      operand2_mnemonic = (width == 0) ? register_lookup_table[reg1] :
                          register_lookup_table_wide[reg1];
    }
    break;
  };

  //- rhjr: instruction construction
  char* str = (char*) arena_alloc(arena, buffer_size);
  char** ptr = &str;

  sprintf(*ptr, "%s %s, %s",
                opcode_mnemonic, operand1_mnemonic, operand2_mnemonic);

  ptr += buffer_size;
  *ptr = '\0';

  output.content = (uint8_t*) str;
  output.length = 10;

  str8_list_push(arena, result, str8_lit("bits 16\r\n", 9));
  str8_list_push(arena, result, output);
}


//- rhjr: eightysix 
int
main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf("(EightyOne) Usage: '%s [DEST FILE] [SOURCE FILE]'\n", argv[0]);
    printf("  - Make sure you have provided a destination and source file.\n");
    printf("  - Please enter a valid file name.\n");
    exit(0);
  }

  Arena arena = {0};
  String8List list = {0};

  arena_init(&arena, &buffer, ARENA_DEFAULT_COMMIT_SIZE);

  Inst *result = (Inst*) io_read_file(&arena, argv[2]);
  inst_decoding(&arena, &list, result);

  String8 output = str8_list_join(&arena, &list);
  io_write_file(argv[1], &output);

	return 0;
}

