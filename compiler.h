#ifndef ROSE_COMPILER_H
#define ROSE_COMPILER_H
#include "vm.h"
#include "object.h"

ObjFunction* compile(const char* source, char* ExePath, char* Dir, bool isPackage);
void markCompilerRoots();

#endif