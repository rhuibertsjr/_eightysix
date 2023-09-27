#ifndef EIGHTYSIX_H
#define EIGHTYSIX_H

#include <stdint.h>
#include <stdlib.h>

#define IO_RETURN_DEFER(value) do { result = value; goto defer; } while(0)

//- rhjr: arena
#define ARENA_DEFAULT_COMMIT_SIZE 1024

typedef struct Arena Arena;
struct Arena {
	uint8_t *buffer;
	size_t offset;
	size_t size;
};

void   arena_init  (Arena *arena, void *buffer, size_t size);
void * arena_alloc (Arena *arena, size_t size);
void   arena_free  (Arena *arena);


//- rhjr: strings
typedef struct String8 String8;
struct String8 {
	uint8_t *content;
	uint8_t length;
};

typedef struct String8Node String8Node;
struct String8Node {
	String8 string;
	String8Node *next;
};

typedef struct String8List String8List;
struct String8List {
	String8Node *first;
	String8Node *last;
	uint8_t node_count; 
	uint8_t total_length; 
};

// rhjr: constructors
String8 str8 (uint8_t *string, size_t length);

#define str8_lit(string, length) str8((uint8_t*)(string), length)

// rhjr: helpers
String8 str8_range   (uint8_t *first_char, uint8_t *last_char);
String8 str8_cstring (uint8_t *cstring);

void    str8_list_push (Arena *arena, String8List *list, String8 string);
String8 str8_list_join (Arena *arena, String8List *list);


//- rhjr: io
void *   io_read_file  (Arena *dst, String8 *file_path);
uint32_t io_write_file (const char *dst, String8 *src);

#endif 
