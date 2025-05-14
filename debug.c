#include <stdio.h>
#include <stdint.h>
#include "debug.h"
#include "value.h"
#include "chunk.h"
#include "object.h"

void disassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset += disassembleInstruction(chunk, offset);
  }
}

static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return 1;
}

static int byteInstruction(const char* name, Chunk* chunk, int offset) {
    int constant = 0;
    int pos = offset + 1;

    constant |= ((int)(chunk->code[pos++]));
    constant |= ((int)(chunk->code[pos++])) << 8;
    constant |= ((int)(chunk->code[pos++])) << 16;
    constant |= ((int)(chunk->code[pos++])) << 24;

    printf("%-16s %4d\n", name, constant);
    int new_offset = 1 + sizeof(int);
    return new_offset;
}

static int jumpInstruction(const char* name, int sign,
    Chunk* chunk, int offset) {
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset,
        offset + 3 + sign * jump);
    return offset + 3;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  return 2;
}

static int longConstantInstruction(const char* name, Chunk* chunk, int offset) {
    int constant = 0;
    int pos = offset + 1;

    constant |= ((int)(chunk->code[pos++]));
    constant |= ((int)(chunk->code[pos++])) << 8;
    constant |= ((int)(chunk->code[pos++])) << 16;
    constant |= ((int)(chunk->code[pos++])) << 24;

    printf("%-16s %d '", name, constant); // fix fail to read bug "debugger bug not comiler"
    if (constant > chunk->constants.count || constant < 0)
        printf("failed to read");
    else
        printValue(chunk->constants.values[constant]);
    printf("'\n");
    int new_offset = 1 + sizeof(int);
    return new_offset;
}

int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);
  printf("%4d ", getLine(chunk, offset));

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    case OP_ARRAY:
      return simpleInstruction("OP_ARRAY", offset);
    case OP_CONSTANT_LONG:
      return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);
    case OP_NEGATE:
        return simpleInstruction("OP_NEGATE", offset);
    case OP_ADD:
        return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return simpleInstruction("OP_DIVIDE", offset);
    case OP_NIL:
        return simpleInstruction("OP_NIL", offset);
    case OP_TRUE:
        return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
        return simpleInstruction("OP_FALSE", offset);
    case OP_NOT:
        return simpleInstruction("OP_NOT", offset);
    case OP_EQUAL:
        return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
        return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
        return simpleInstruction("OP_LESS", offset);
    case OP_PRINT:
        return simpleInstruction("OP_PRINT", offset);
    case OP_POP:
        return simpleInstruction("OP_POP", offset);
    case OP_DEFINE_GLOBAL:
        return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_GET_GLOBAL:
        return constantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
        return constantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OP_GET_LOCAL:
        return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
        return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case OP_JUMP:
        return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
        return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    case OP_IMPORT:
        return simpleInstruction("OP_IMPORT", offset);
    case OP_INCLUDE:
        return simpleInstruction("OP_INCLUDE", offset);
    case OP_LOOP:
        return jumpInstruction("OP_LOOP", -1, chunk, offset);
    case OP_CALL:
        return byteInstruction("OP_CALL", chunk, offset);
    case OP_CLOSURE: {
        return constantInstruction("OP_CLOSURE", chunk, offset);
    }
    case OP_GET_UPVALUE:
        return byteInstruction("OP_GET_UPVALUE", chunk, offset);
    case OP_SET_UPVALUE:
        return byteInstruction("OP_SET_UPVALUE", chunk, offset);
    case OP_CLOSE_UPVALUE:
        return simpleInstruction("OP_CLOSE_UPVALUE", offset);
    case OP_CLASS:
        return constantInstruction("OP_CLASS", chunk, offset);
    case OP_GET_PROPERTY:
        return constantInstruction("OP_GET_PROPERTY", chunk, offset);
    case OP_SET_PROPERTY:
        return constantInstruction("OP_SET_PROPERTY", chunk, offset);
    case OP_METHOD:
        return constantInstruction("OP_METHOD", chunk, offset);
    case OP_INVOKE:
        return constantInstruction("OP_INVOKE", chunk, offset);
    case OP_INHERIT:
        return simpleInstruction("OP_INHERIT", offset);
    case OP_GET_SUPER:
        return constantInstruction("OP_GET_SUPER", chunk, offset);
    case OP_SUPER_INVOKE:
        return constantInstruction("OP_SUPER_INVOKE", chunk, offset);
    default:
      printf("Unknown opcode %d\n", instruction);
      return 1;
  }
}

void printChunk(Chunk* chunk, char* name){
    printValueArray(&chunk->constants);
    printf("[Line Buffer, count '%i',  capacity '%i']\n[", chunk->lineCount, chunk->lineCapacity);
    for(int i = 0; i < chunk->lineCount; i++){
        printf("%i", chunk->lines[i]);
        if(i != chunk->lineCount-1)
            printf(" | ");
    }
    printf("]\n");
    /////////////////////
    printRawChunk(chunk, name);
    disassembleChunk(chunk, name);
}

void printRawChunk(Chunk* chunk, char* name){
    printf("[Raw Chunk, count '%i',  capacity '%i']\n[ ", chunk->count, chunk->capacity);
    for(int i = 0; i < chunk->count; i++){
        uint8_t instruction = chunk->code[i];
        printf("%03i, ", instruction);
    }
    printf("]\n");
}