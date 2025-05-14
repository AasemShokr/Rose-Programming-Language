#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void repl();
static char* readFile(const char* path);
static void runFile(const char* path);

int main(int argc, const char* argv[]){

    // init the vm
    initVM();

    if (argc == 1) {
        repl();
    }
    else if (argc == 2) {
        runFile(argv[1]);
    }
    else {
        fprintf(stderr, "Usage: rose [path]\n");
        exit(64);
    }

    //runFile("test.rose");

    freeVM();
    return 0;
}

// colored text
static void red() {
    printf("\033[1;31m");
}

static void yellow() {
  printf("\033[1;33m");
}

static void green() {
  printf("\033[0;32m");
}

static void blue() {
  printf("\033[0;34m");
}

static void reset() {
    printf("\033[0m");
}


static void repl() {
    red();
    printf("ROSE Language [Version 1.0]\n");
    yellow();
    printf("(C) Created and designed by ");
    green();
    printf("Aasem Ibrahim Shokr\n");
    reset();

    char line[1024];
    for (;;) {
        blue();
        printf(">> ");
        reset();

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    
    // fail to open the file for some reason
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    // check if succeded
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    // terminate the string
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}