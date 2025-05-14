#include <stdint.h>
#include "chunk.h"
#include "memory.h"
#include "vm.h"

void initChunk(Chunk* chunk){
    initValueArray(&chunk->constants);
    chunk->code = NULL;
    chunk->lines = NULL;
    chunk->count = 0;
    chunk->lineCount = 0;
    chunk->lineCapacity = 0;
    chunk->capacity = 0;
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if(chunk->capacity < chunk->count + 1){
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }
    /*if (chunk->lineCapacity < chunk->lineCount + 1 || chunk->lineCapacity <= line) {
        int cap = chunk->lineCapacity;
        chunk->lineCapacity = GROW_CAPACITY(cap);
        if (chunk->lines < 0) {
            int i = 0;
        }
        chunk->lines = GROW_ARRAY(int, chunk->lines, cap, chunk->lineCapacity);
    }*/
    // write the byte
    chunk->code[chunk->count] = byte;
    chunk->count++;
    // write line number
    /*if (chunk->lineCount < line) {
        chunk->lines[line-1] = 1;
        chunk->lineCount = line;
    }
    else{
        chunk->lines[line-1]++;
    }*/
}

void writeInt(Chunk* chunk, unsigned char opcode, int value, int line) {
    // write a 3 byte operand
    if (chunk->capacity < chunk->count + 5) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }
    /*if (chunk->lineCapacity < chunk->lineCount + 1 || chunk->lineCapacity <= line) {
        int cap = chunk->lineCapacity;
        chunk->lineCapacity = GROW_CAPACITY(cap);
        if (chunk->lines < 0) {
            int i = 0;
        }
        chunk->lines = GROW_ARRAY(int, chunk->lines, cap, chunk->lineCapacity);
    }*/

    int constant_index = value;

    // write the byte
    chunk->code[chunk->count] = opcode;
    chunk->count++;

    // encode int in char array
    uint8_t* op_pointer = &(chunk->code[chunk->count]);
    op_pointer[0] = constant_index & 0xFF;
    op_pointer[1] = (constant_index >> 8) & 0xFF;
    op_pointer[2] = (constant_index >> 16) & 0xFF;
    op_pointer[3] = (constant_index >> 24) & 0xFF;

    chunk->count += sizeof(int);

    // write line number
    /*if (chunk->lineCount < line) {
        chunk->lines[line - 1] = 1 + sizeof(int);
        chunk->lineCount = line;
    }
    else {
        // add one operand
        chunk->lines[line - 1] += 1 + sizeof(int);
    }*/
}

void writeConstant(Chunk* chunk, unsigned char opcode, Value value, int line){
    // write a 3 byte operand
    if(chunk->capacity < chunk->count + 5){
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }
    /*if (chunk->lineCapacity < chunk->lineCount + 1 || chunk->lineCapacity <= line) {
        int cap = chunk->lineCapacity;
        chunk->lineCapacity = GROW_CAPACITY(cap);
        if (chunk->lines < 0) {
            int i = 0;
        }
        chunk->lines = GROW_ARRAY(int, chunk->lines, cap, chunk->lineCapacity);
    }*/

    int constant_index = addConstant(chunk, value);

    // write the byte
    chunk->code[chunk->count] = opcode;
    chunk->count++;

    // encode int in char array
    uint8_t* op_pointer = &(chunk->code[chunk->count]);
    op_pointer[0] =  constant_index        & 0xFF;
    op_pointer[1] = (constant_index >> 8)  & 0xFF;
    op_pointer[2] = (constant_index >> 16) & 0xFF;
    op_pointer[3] = (constant_index >> 24) & 0xFF;

    chunk->count += sizeof(int);

    // write line number
    /*if (chunk->lineCount < line) {
        chunk->lines[line-1] = 1 + sizeof(int);
        chunk->lineCount = line;
    }
    else{
        // add one operand
        chunk->lines[line-1] += 1 + sizeof(int);
    }*/
}

void freeChunk(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  //FREE_ARRAY(int, chunk->lines, chunk->capacity);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value) {
  push(value); // to protect object from the garbage collection monster
  writeValueArray(&chunk->constants, value);
  pop();
  return chunk->constants.count - 1;
}

int getLine(Chunk* chunk, int offset){
    /*int counter = 0;
    for(int i = 0; i < *chunk->lines; i++){
        if(offset < counter){
            return i;
        }
        else{
            counter += chunk->lines[i];
        }
    }*/
    return 1; // error occured
}