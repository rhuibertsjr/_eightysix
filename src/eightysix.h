#ifndef EIGHTYSIX_H
#define EIGHTYSIX_H

#include <stdint.h>

#ifdef DEBUG
    #define ASSERT(cond) do { if (!(cond)) exit(0); } while(0)
#else
    #define ASSERT(cond)
#endif

#define IO_RETURN_DEFER(value) do { result = value; goto defer; } while(0)

//
// Strings
//
typedef struct String String;
struct String {
	uint8_t *string;
	size_t size;
};

String string(uint8_t *string, size_t length);

// Helper functions
String string_range(uint8_t *first_char, uint8_t *last_char);
String cstring_to_string(uint8_t *cstring);

//
// Arena
//
typedef struct Arena Arena;
struct Arena {
	void *back_buffer;
	size_t offset;
	size_t size;
};

void   arena_init(Arena *dst, void *src, size_t size);
void * arena_alloc(Arena *arena, size_t block_size);
void   arena_free(Arena *arena);

//
// I/O
//
uint32_t * io_read_file(Arena *dst, String *file_path);
uint32_t   io_write_file(String *dst, void *src);

#endif 
