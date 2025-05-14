#ifndef ROSE_CHUNK_H
#define ROSE_CHUNK_H

#include "value.h"

typedef enum{
    OP_CONSTANT_LONG,
    OP_NOT,
    // Data types
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    // Arithmetic operations
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE, 
    // Other
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    // native
    OP_PRINT,
    OP_POP,
    OP_IMPORT,
    OP_INCLUDE,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,
    OP_CALL,
    OP_CLOSURE,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_CLOSE_UPVALUE,
    OP_CLASS,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_METHOD,
    OP_INVOKE,
    OP_INHERIT,
    OP_GET_SUPER,
    OP_SUPER_INVOKE,
    OP_ARRAY,
    // return
    OP_RETURN
} OpCode;

typedef struct {
    uint8_t* code;
    int count;
    int capacity;
    int* lines;
    int lineCount;
    int lineCapacity;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void writeConstant(Chunk* chunk, unsigned char opcode, Value value, int line);
void writeInt(Chunk* chunk, unsigned char opcode, int value, int line);
int addConstant(Chunk* chunk, Value value);
int getLine(Chunk* chunk, int offset);

#endif