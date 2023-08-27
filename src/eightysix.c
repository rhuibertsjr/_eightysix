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
	String result;
	result.length = (uint64_t)(last_char - first_char);
	strncpy(&result.content, *first_char, result.length);

	return result; 
}

String string_cstring(const char *cstring)
{
	uint8_t *ptr = cstring;
	for(; *ptr != 0; ptr += 1);
	return string_range(cstring, ptr);
}

void string_list_push (Arena *arena, StringList *list, String string)
{
	ASSERT(string.length != 0);
	ASSERT(string.length <= 32);

	StringNode node      = {0};
	uint8_t    node_size = sizeof(StringNode);

	node.string.length  = string.length;

	strncpy(&node.string.content, &string.content, string.length);

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

String string_list_join(Arena *arena, StringList *list)
{
	String result = {0};

	uint8_t length = list->total_length + (list->node_count * 2);

	// NOTE(rhjr): Total length + joint character '\r\n' (Windows).
	uint8_t *str =
		(uint8_t*) arena_alloc(arena, length);
	uint8_t *ptr = str; // NOTE(rhjr) Used to iterate. 

	for(StringNode *current  = list->first;
		current != NULL;
		current  = current->next)
	{
		memcpy(ptr, current->string.content, current->string.length);
		ptr += current->string.length;

		//next line
		*ptr++ = (uint8_t) '\r';
		*ptr   = (uint8_t) '\n';
	}

	*ptr++ = (uint8_t) '\0';

	memcpy(&result.content, str, length);
	result.length = list->total_length;

	return result;
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

uint32_t io_write_file (const char *dst, String *src)
{
	FILE    *file   = NULL; 
	uint32_t result = 0;

	file = fopen(dst, "wb");
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
	arena_init(&arena, &buffer, BUFFER_SIZE / 2);

	Arena write = {0};
	arena_init(&write, &buffer[BUFFER_SIZE / 2], BUFFER_SIZE / 2);

	memset(&buffer, 0, BUFFER_SIZE);

	StringList list = { 0 };
	String str1   = { "Daar ga je man", 14 };
	String str2   = { "TEST 2", 6 };
	String str3   = { "TEST 3", 6 };

	string_list_push(&arena, &list, str1);
	string_list_push(&arena, &list, str2);
	string_list_push(&arena, &list, str3);

	for(StringNode *current = list.first;
		current != NULL;
		current  = current->next)
	{
		printf("List content: ");
		for (uint8_t i = 0; i < current->string.length; i++) {
			printf("%c", current->string.content[i]);
		}
		printf("\n");

		printf("List size: %d", current->string.length);

		printf("\n");
	}

	String result = string_list_join(&write, &list);

	io_write_file("test.txt", &result);

	return 0;
}

