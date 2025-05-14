#ifndef ROSE_DEBUG_H
#define ROSE_DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);
void printChunk(Chunk* chunk, char* name);
void printRawChunk(Chunk* chunk, char* name);

#endif