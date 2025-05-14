#include "io.h"
#include "../../value.h"
#include "../../vm.h"
#include "../../object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// File I/O
void binary_string_to_bytes(const char* binary_string, unsigned char* bytes, size_t* byte_count) {
    size_t string_length = strlen(binary_string);
    *byte_count = (string_length + 7) / 8;  // Calculate number of bytes needed

    for (size_t i = 0; i < *byte_count; i++) {
        unsigned char byte = 0;
        for (int j = 0; j < 8; j++) {
            size_t index = i * 8 + j;
            if (index < string_length) {
                byte = (byte << 1) | (binary_string[index] - '0');
            }
            else {
                byte <<= 1;  // Pad with 0 if string is not a multiple of 8
            }
        }
        bytes[i] = byte;
    }
}

int write_bytes_to_file(const char* filename, const unsigned char* bytes, size_t byte_count) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    size_t written = fwrite(bytes, 1, byte_count, file);

    if (written != byte_count) {
        perror("Error writing to file");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

static Value writeBinaryStringToFile(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) return NIL_VAL;

    const char* filename = AS_CSTRING(args[0]);
    const char* binary_string = AS_CSTRING(args[1]);
    size_t string_length = strlen(binary_string);

    // Calculate the number of bytes needed (always at least 1)
    size_t byte_count = (string_length + 7) / 8;
    if (byte_count == 0) byte_count = 1;

    unsigned char* bytes = (unsigned char*)malloc(byte_count);
    if (bytes == NULL) {
        fprintf(stderr, "Not enough memory to allocate buffer.\n");
        return NIL_VAL;
    }

    // Convert binary string to bytes
    for (size_t i = 0; i < byte_count; i++) {
        unsigned char byte = 0;
        for (int j = 0; j < 8 && (i * 8 + j) < string_length; j++) {
            if (binary_string[i * 8 + j] == '1') {
                byte |= (1 << (7 - j));
            }
            else if (binary_string[i * 8 + j] != '0') {
                fprintf(stderr, "Invalid character in binary string.\n");
                free(bytes);
                return NIL_VAL;
            }
        }
        bytes[i] = byte;
    }

    // Write bytes to file
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\" for writing.\n", filename);
        free(bytes);
        return NIL_VAL;
    }

    size_t written = fwrite(bytes, 1, byte_count, file);
    fclose(file);
    free(bytes);

    if (written != byte_count) {
        fprintf(stderr, "Could not write all data to file \"%s\".\n", filename);
        return NIL_VAL;
    }

    return BOOL_VAL(true);
}

// read a text file
static Value readFile(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_STRING(args[0])) return NIL_VAL;

    const char* path = AS_CSTRING(args[0]);
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

    Value string = OBJ_VAL(takeString(buffer, fileSize + 1, true));

    return string;
}

// write a text file
static Value writeFile(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    if (!IS_STRING(args[0])) return NIL_VAL;
    if (!IS_STRING(args[1])) return NIL_VAL;

    const char* name = AS_CSTRING(args[0]);
    const char* content = AS_CSTRING(args[1]);

    FILE* filePointer;
    filePointer = fopen(name, "w");
    fprintf(filePointer, "%s", content);

    fclose(filePointer);
    return NIL_VAL;
}
// Console I/O
// Print Line
static Value Println(int argCount, Value* args) {
    for (int i = 0; i < argCount; i++) {
        printValue(args[i]);
        printf("\n");
    }
    return NIL_VAL;
}
// Print Colored text
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

bool stringEquals(const char* a, const char* b) {
    return !strcmp(a, b);
}

static Value PrintColored(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    if (!IS_STRING(args[0])) return NIL_VAL;

    const char* color = AS_CSTRING(args[0]);

    if (stringEquals(color, "red"))
        red();
    else if (stringEquals(color, "yellow"))
        yellow();
    else if (stringEquals(color, "green"))
        green();
    else if (stringEquals(color, "blue"))
        blue();

    printValue(args[1]);

    reset();
    
    return NIL_VAL;
}
// Print Line Colored
static Value PrintLnColored(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    if (!IS_STRING(args[0])) return NIL_VAL;

    const char* color = AS_CSTRING(args[0]);

    if (stringEquals(color, "red"))
        red();
    else if (stringEquals(color, "yellow"))
        yellow();
    else if (stringEquals(color, "green"))
        green();
    else if (stringEquals(color, "blue"))
        blue();

    printValue(args[1]);
    printf("\n");
    reset();
    return NIL_VAL;
}
/////////////////////////////////////////////////////////////////////////////////

void LoadIO() {
    // file io
    defineNative("sys_lib_io_open", readFile);
    defineNative("sys_lib_io_write", writeFile);
    defineNative("sys_lib_io_write_binary_string", writeBinaryStringToFile);
    // open to stream
    // write text
    // write binary
    // close stream
    // cmd io
    defineNative("println", Println);
    defineNative("printc", PrintColored);
    defineNative("printlnc", PrintLnColored);
}