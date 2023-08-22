#include "eightysix.h"

#include <stdio.h>
#include <string.h>

//
// String 
//
String string(uint8_t *string, size_t length)
{
	String str = { string, length };
	return str;
}

String string_range(uint8_t *first_char, uint8_t *last_char)
{
	String str = { first_char, (uint64_t)(last_char - first_char)};
	return str;
}

String cstring_to_string(uint8_t *cstring)
{
	uint8_t *ptr = cstring;
	for(; *ptr != 0; ptr += 1);
	return string_range(cstring, ptr);
}

//
// Memory
//
void arena_init(Arena *dst, void *src, size_t size)
{
	ASSERT(size > 0);

	dst->back_buffer = src;
	dst->size        = size;
	dst->offset      = 0;
}

void * arena_alloc(Arena *arena, size_t block_size)
{
	ASSERT(block_size > 0);

	if (arena->offset + block_size <= arena->size) {
		void *ptr = (uint64_t) arena->back_buffer + arena->offset;
		arena->offset += block_size;

		memset(ptr, 0, block_size);
		return ptr;
	}

	return NULL;
}

void arena_free(Arena *arena)
{
	arena->offset = 0;
}

//
// I/O
//
uint32_t * io_read_file(Arena *dst, String *file_path)
{
	// TODO(rene) Safe the contents of the file_path into an String object.
	ASSERT(file_path->size > 0);

	FILE    *file = NULL;
	uint32_t file_length = 0;

	uint8_t *result  = NULL;

	file = fopen(file_path->string, "rb");
	if (file == NULL)
		IO_RETURN_DEFER(NULL);

	fseek(file, 0, SEEK_END);
    file_length = ftell(file);
	fseek(file, 0, SEEK_SET);

	result = arena_alloc(dst, file_length);
	
	uint32_t processed_chars =
		fread(result, sizeof(uint8_t), file_length, file);
	if (processed_chars == 0)
		IO_RETURN_DEFER(NULL);

defer:
    fclose(file);
	return result;
}

uint32_t io_write_file(String *dst, void *src)
{
	ASSERT(dst->size > 0);

	FILE    *file   = NULL; 
	uint32_t result = 0;

	file = fopen(dst->string, "wb");
	if (file == NULL)
		IO_RETURN_DEFER(-1);

	result = fwrite(src, sizeof(uint8_t), strlen(src), file);
	if (result == 0) 
		IO_RETURN_DEFER(-1);

defer:
    fclose(file);
	return result;
}

//
// 8086 
//
int main(int argc, char *argv[])
{
	if (argc < 2) {
		exit(0);
	}

	const uint8_t buffer[1024];

	String file = cstring_to_string(argv[1]);
	String out  = cstring_to_string(argv[2]);

	Arena arena = {0};
	arena_init(&arena, &buffer, 1024);

	uint32_t *content = io_read_file(&arena, &file);

	uint8_t result = io_write_file(&out, content);

	return 0;
}
