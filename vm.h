#ifndef ROSE_VM_H
#define ROSE_VM_H

#include "compiler.h"
#include "chunk.h"
#include "value.h"
#include "table.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
	ObjClosure* closure;
	uint8_t* ip;
	Value* slots;
} CallFrame;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct {
	CallFrame frames[FRAMES_MAX];
	int frameCount;
	// stack
	Value stack[STACK_MAX];
	Value* stackTop;
	// objects
	ObjUpvalue* openUpvalues;
	Obj* objects;
	Table strings;
	Table globals;
	// garbage collection
	int grayCount;
	int grayCapacity;
	Obj** grayStack;
	size_t bytesAllocated;
	size_t nextGC;
	// OOP
	ObjString* initString;
	ObjString* destString;
} VM;

extern VM vm;

InterpretResult interpret(const char* source);
void initVM();
void freeVM();
void defineNative(const char* name, NativeFn function);
void push(Value value);
Value pop();
bool callDestructor(ObjInstance* instance);

#endif