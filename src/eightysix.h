
#ifndef EIGHTYSIX_H
#define EIGHTYSIX_H

#include <stdint.h>
#include <stdlib.h>

#ifdef DEBUG
    #define ASSERT(cond) do { if (!(cond)) exit(0); } while(0)
#else
    #define ASSERT(cond)
#endif

#define IO_RETURN_DEFER(value) do { result = value; goto defer; } while(0)

//
// Arena
//
typedef struct Arena Arena;
struct Arena {
	uint8_t *buffer;
	size_t offset;
	size_t size;
};

void   arena_init  (Arena *arena, void *buffer, size_t size);
void * arena_alloc (Arena *arena, size_t size);
void   arena_free  (Arena *arena);

//
// Strings
//     - NOTE Should be treated as an immutable type.
//
typedef struct String String;
struct String {
	uint8_t *content;
	uint8_t length;
};

#define StringNodeSize 12

typedef struct StringNode StringNode;
struct StringNode {
	String string;
	StringNode *next;
	uint8_t padding;
};

typedef struct StringList StringList;
struct StringList {
	StringNode *first;
	StringNode *last;
	size_t node_count; 
	size_t total_length; 
};

String string (uint8_t *string, size_t length);

// Helper functions
String string_range   (uint8_t *first_char, uint8_t *last_char);
String string_cstring (uint8_t *cstring);

void   string_list_push (Arena *arena, StringList *list, String string);
String string_list_join (Arena *arena, StringList *list);

//
// I/O
//
void *   io_read_file  (Arena *dst, String *file_path);
uint32_t io_write_file (String *dst, String *src);

//
// 8086 Machine instruction 
//
typedef struct Instf Instf;
struct Instf {
	uint8_t opcode;    // See Table 4-20 (8086 manual).
	uint8_t flags;
};

// Instruction opcode (Table 4-12). 
#define OPC_MOV 0b100010

// Instruction single-bit field (Table 4-8).


// Instruction decoding
String instr_decode(Instf instruction);

//
// Registers
//
typedef enum Reg Reg;
enum Reg {
	REG_A = 0x0,
	REG_B,
	REG_C,
	REG_END
};

#endif 
