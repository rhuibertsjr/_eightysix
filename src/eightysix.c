#include "eightysix.h"

#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024 

uint8_t buffer[BUFFER_SIZE];

//
// String 
//
String string(uint8_t *content, uint8_t length)
{
	String result = { content, length };
	return result; 
}

String string_range(uint8_t *first_char, uint8_t *last_char)
{
	String result = { first_char, (uint64_t)(last_char - first_char) };
	return result; 
}

String string_cstring(uint8_t *cstring)
{
	uint8_t *ptr = cstring;
	for(; *ptr != 0; ptr += 1);
	return string_range(cstring, ptr);
}

void string_list_push (Arena *arena, StringList *list, String string)
{
	// TODO(rhjr): Now if the string that should be used gets out of scope,
	//then the pointer to that string is dangeling.

	StringNode node = {0};
	node.string.content = string.content;
	node.string.length  = string.length;

	printf("String content: %s\n", node.string.content);
	printf("String size: %d\n",    node.string.length);

	uint8_t node_size = sizeof(StringNode);

	uint8_t *ptr = (uint8_t*) arena_alloc(arena, node_size);
	if (list->first == NULL) 
		list->first = list->last = ptr;
	else {
		list->last->next = ptr;
		list->last = ptr;
	}
		
	memcpy(ptr, &node, node_size);

	list->total_length += string.length;
	list->node_count   += 1;
}

//
// Memory
//
void arena_init(Arena *arena, void *buffer, size_t size)
{
	ASSERT(size > 0);

	arena->buffer = (uint8_t*) buffer;
	arena->size   = size;
	arena->offset = 0;
}

void * arena_alloc(Arena *arena, size_t size)
{
	ASSERT(size > 0);

	// TODO(rhjr): Fix memory alignment
	if (arena->offset + size <= arena->size) {
		void *ptr = (uint8_t*) arena->buffer + arena->offset;
		arena->offset += size;

		memset(ptr, 0, size);
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
void * io_read_file(Arena *dst, String *src)
{
	ASSERT(src->length > 0);

	FILE    *file        = NULL;
	uint32_t file_length = 0;
	void    *result      = NULL;

	file = fopen(src->content, "rb");
	if (file == NULL)
		IO_RETURN_DEFER(NULL);

	fseek(file, 0, SEEK_END);
    file_length = ftell(file);
	fseek(file, 0, SEEK_SET);

	result = (void*) arena_alloc(dst, file_length);
	
	uint32_t processed_chars =
		fread(result, sizeof(uint8_t), file_length, file);
	if (processed_chars == 0)
		IO_RETURN_DEFER(NULL);

defer:
    fclose(file);
	return result;
}	

uint32_t io_write_file(String *dst, String *src)
{
	ASSERT(dst->length > 0);

	FILE    *file   = NULL; 
	uint32_t result = 0;

	file = fopen(dst->content, "wb");
	if (file == NULL)
		IO_RETURN_DEFER(-1);

	result = fwrite(src->content, sizeof(uint8_t), src->length, file);
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
	Arena arena = {0};
	arena_init(&arena, &buffer, BUFFER_SIZE);

	memset(&buffer, 0, BUFFER_SIZE);

	StringList list = { 0 };
	String str1   = { "TEST 1", 105 };
	String str2   = { "TEST 2", 6 };
	String str3   = { "TEST 3", 6 };

	string_list_push(&arena, &list, str1);
	string_list_push(&arena, &list, str2);
	string_list_push(&arena, &list, str3);

	for(StringNode *current = list.first;
		current != NULL;
		current  = current->next)
	{
		printf("List content: %s", current->string.content);
		printf("List size: %d", current->string.length);

		printf("\n");
	}

	return 0;
}

