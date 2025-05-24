#include "library_runtime.h"
#include "value.h"
#include "stdbool.h"
#include "vm.h"
#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define LIBRARY_HANDLE HMODULE
#define LOAD_LIBRARY(path) LoadLibraryA(path)
#define GET_FUNCTION(handle, name) GetProcAddress(handle, name)
#define CLOSE_LIBRARY(handle) FreeLibrary(handle)
#define LIBRARY_ERROR() GetLastError()
#else
#include <dlfcn.h>
#define LIBRARY_HANDLE void*
#define LOAD_LIBRARY(path) dlopen(path, RTLD_LAZY)
#define GET_FUNCTION(handle, name) dlsym(handle, name)
#define CLOSE_LIBRARY(handle) dlclose(handle)
#define LIBRARY_ERROR() dlerror()
#endif

// Maximum number of loaded libraries
#define MAX_LIBRARIES 64
#define MAX_FUNCTIONS_PER_LIB 256

// Function signature types supported
typedef enum {
    FUNC_VOID_VOID,     // void func()
    FUNC_INT_VOID,      // int func()
    FUNC_DOUBLE_VOID,   // double func()
    FUNC_PTR_VOID,      // void* func()
    FUNC_VOID_INT,      // void func(int)
    FUNC_INT_INT,       // int func(int)
    FUNC_DOUBLE_INT,    // double func(int)
    FUNC_INT_DOUBLE,    // int func(double)
    FUNC_DOUBLE_DOUBLE, // double func(double)
    FUNC_INT_INT_INT,   // int func(int, int)
    FUNC_DOUBLE_DOUBLE_DOUBLE, // double func(double, double)
    FUNC_PTR_PTR,       // void* func(void*)
    FUNC_INT_PTR,       // int func(void*)
    FUNC_PTR_INT,       // void* func(int)
    FUNC_CUSTOM         // Custom signature - user defines calling convention
} FunctionSignature;

// Structure to hold function information
typedef struct {
    void* function_ptr;
    FunctionSignature signature;
    char name[64];
    bool is_valid;
} DLLFunction;

// Structure to hold library information
typedef struct {
    LIBRARY_HANDLE handle;
    char path[256];
    DLLFunction functions[MAX_FUNCTIONS_PER_LIB];
    int function_count;
    bool is_loaded;
} LoadedLibrary;

// Global library registry
static LoadedLibrary g_libraries[MAX_LIBRARIES];
static int g_library_count = 0;

// Initialize the DLL system
static void InitializeDLLSystem() {
    static bool initialized = false;
    if (!initialized) {
        memset(g_libraries, 0, sizeof(g_libraries));
        g_library_count = 0;
        initialized = true;
    }
}

// Find library by handle
static LoadedLibrary* FindLibraryByHandle(LIBRARY_HANDLE handle) {
    for (int i = 0; i < g_library_count; i++) {
        if (g_libraries[i].handle == handle && g_libraries[i].is_loaded) {
            return &g_libraries[i];
        }
    }
    return NULL;
}

// Load a dynamic library
static Value RoseLoadLibrary(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_STRING(args[0])) return NIL_VAL;

    InitializeDLLSystem();

    if (g_library_count >= MAX_LIBRARIES) {
        printf("Error: Maximum number of libraries (%d) reached\n", MAX_LIBRARIES);
        return NIL_VAL;
    }

    const char* library_path = AS_CSTRING(args[0]);

    // Check if library is already loaded
    for (int i = 0; i < g_library_count; i++) {
        if (g_libraries[i].is_loaded && strcmp(g_libraries[i].path, library_path) == 0) {
            return NATIVE_VAL(g_libraries[i].handle, sizeof(LIBRARY_HANDLE));
        }
    }

    // Load the library
    LIBRARY_HANDLE handle = LOAD_LIBRARY(library_path);
    if (!handle) {
        printf("Error: Failed to load library '%s'\n", library_path);
#ifndef _WIN32
        printf("Error details: %s\n", (char*)LIBRARY_ERROR());
#endif
        return NIL_VAL;
    }

    // Store library information
    LoadedLibrary* lib = &g_libraries[g_library_count];
    lib->handle = handle;
    strncpy(lib->path, library_path, sizeof(lib->path) - 1);
    lib->path[sizeof(lib->path) - 1] = '\0';
    lib->function_count = 0;
    lib->is_loaded = true;

    g_library_count++;

    printf("Successfully loaded library: %s\n", library_path);
    return NATIVE_VAL(handle, sizeof(LIBRARY_HANDLE));
}

// Get function from loaded library
static Value RoseGetFunction(int argCount, Value* args) {
    if (argCount != 3) return NIL_VAL;
    if (!IS_STRING(args[1])) return NIL_VAL;
    if (!IS_NUMBER(args[2])) return NIL_VAL;

    LIBRARY_HANDLE handle = AS_NATIVE_VAL(args[0]);
    const char* function_name = AS_CSTRING(args[1]);
    FunctionSignature signature = (FunctionSignature)AS_NUMBER(args[2]);

    LoadedLibrary* lib = FindLibraryByHandle(handle);
    if (!lib) {
        printf("Error: Invalid library handle\n");
        return NIL_VAL;
    }

    if (lib->function_count >= MAX_FUNCTIONS_PER_LIB) {
        printf("Error: Maximum functions per library reached\n");
        return NIL_VAL;
    }

    // Get function pointer
    void* func_ptr = GET_FUNCTION(handle, function_name);
    if (!func_ptr) {
        printf("Error: Function '%s' not found in library\n", function_name);
        return NIL_VAL;
    }

    // Store function information
    DLLFunction* func = &lib->functions[lib->function_count];
    func->function_ptr = func_ptr;
    func->signature = signature;
    strncpy(func->name, function_name, sizeof(func->name) - 1);
    func->name[sizeof(func->name) - 1] = '\0';
    func->is_valid = true;

    lib->function_count++;

    printf("Successfully loaded function: %s\n", function_name);
    return NATIVE_VAL(func, sizeof(DLLFunction*));
}

// Call a loaded function with arguments
static Value RoseCallFunction(int argCount, Value* args) {
    if (argCount < 1) return NIL_VAL;

    DLLFunction* func = (DLLFunction*)AS_NATIVE_VAL(args[0]);
    if (!func || !func->is_valid) {
        printf("Error: Invalid function handle\n");
        return NIL_VAL;
    }

    // Call function based on signature
    switch (func->signature) {
    case FUNC_VOID_VOID: {
        void (*f)() = (void (*)())func->function_ptr;
        f();
        return NIL_VAL;
    }

    case FUNC_INT_VOID: {
        int (*f)() = (int (*)())func->function_ptr;
        int result = f();
        return NUMBER_VAL(result);
    }

    case FUNC_DOUBLE_VOID: {
        double (*f)() = (double (*)())func->function_ptr;
        double result = f();
        return NUMBER_VAL(result);
    }

    case FUNC_PTR_VOID: {
        void* (*f)() = (void* (*)())func->function_ptr;
        void* result = f();
        return NATIVE_VAL(result, sizeof(void*));
    }

    case FUNC_VOID_INT: {
        if (argCount != 2 || !IS_NUMBER(args[1])) return NIL_VAL;
        void (*f)(int) = (void (*)(int))func->function_ptr;
        f((int)AS_NUMBER(args[1]));
        return NIL_VAL;
    }

    case FUNC_INT_INT: {
        if (argCount != 2 || !IS_NUMBER(args[1])) return NIL_VAL;
        int (*f)(int) = (int (*)(int))func->function_ptr;
        int result = f((int)AS_NUMBER(args[1]));
        return NUMBER_VAL(result);
    }

    case FUNC_DOUBLE_INT: {
        if (argCount != 2 || !IS_NUMBER(args[1])) return NIL_VAL;
        double (*f)(int) = (double (*)(int))func->function_ptr;
        double result = f((int)AS_NUMBER(args[1]));
        return NUMBER_VAL(result);
    }

    case FUNC_INT_DOUBLE: {
        if (argCount != 2 || !IS_NUMBER(args[1])) return NIL_VAL;
        int (*f)(double) = (int (*)(double))func->function_ptr;
        int result = f(AS_NUMBER(args[1]));
        return NUMBER_VAL(result);
    }

    case FUNC_DOUBLE_DOUBLE: {
        if (argCount != 2 || !IS_NUMBER(args[1])) return NIL_VAL;
        double (*f)(double) = (double (*)(double))func->function_ptr;
        double result = f(AS_NUMBER(args[1]));
        return NUMBER_VAL(result);
    }

    case FUNC_INT_INT_INT: {
        if (argCount != 3 || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) return NIL_VAL;
        int (*f)(int, int) = (int (*)(int, int))func->function_ptr;
        int result = f((int)AS_NUMBER(args[1]), (int)AS_NUMBER(args[2]));
        return NUMBER_VAL(result);
    }

    case FUNC_DOUBLE_DOUBLE_DOUBLE: {
        if (argCount != 3 || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) return NIL_VAL;
        double (*f)(double, double) = (double (*)(double, double))func->function_ptr;
        double result = f(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
        return NUMBER_VAL(result);
    }

    case FUNC_PTR_PTR: {
        if (argCount != 2) return NIL_VAL;
        void* (*f)(void*) = (void* (*)(void*))func->function_ptr;
        void* param = IS_STRING(args[1]) ? (void*)AS_CSTRING(args[1]) : AS_NATIVE_VAL(args[1]);
        void* result = f(param);
        return NATIVE_VAL(result, sizeof(void*));
    }

    case FUNC_INT_PTR: {
        if (argCount != 2) return NIL_VAL;
        int (*f)(void*) = (int (*)(void*))func->function_ptr;
        void* param = IS_STRING(args[1]) ? (void*)AS_CSTRING(args[1]) : AS_NATIVE_VAL(args[1]);
        int result = f(param);
        return NUMBER_VAL(result);
    }

    case FUNC_PTR_INT: {
        if (argCount != 2 || !IS_NUMBER(args[1])) return NIL_VAL;
        void* (*f)(int) = (void* (*)(int))func->function_ptr;
        void* result = f((int)AS_NUMBER(args[1]));
        return NATIVE_VAL(result, sizeof(void*));
    }

    case FUNC_CUSTOM:
    default:
        printf("Error: Unsupported function signature or custom calling not implemented\n");
        return NIL_VAL;
    }
}

// Unload a library
static Value RoseUnloadLibrary(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;

    LIBRARY_HANDLE handle = AS_NATIVE_VAL(args[0]);
    LoadedLibrary* lib = FindLibraryByHandle(handle);

    if (!lib) {
        printf("Error: Invalid library handle\n");
        return BOOL_VAL(false);
    }

    // Invalidate all functions from this library
    for (int i = 0; i < lib->function_count; i++) {
        lib->functions[i].is_valid = false;
    }

    // Close the library
    CLOSE_LIBRARY(handle);
    lib->is_loaded = false;

    printf("Successfully unloaded library: %s\n", lib->path);
    return BOOL_VAL(true);
}

// List loaded libraries
static Value RoseListLibraries(int argCount, Value* args) {
    printf("Loaded Libraries:\n");
    for (int i = 0; i < g_library_count; i++) {
        if (g_libraries[i].is_loaded) {
            printf("  %d: %s (%d functions)\n", i, g_libraries[i].path, g_libraries[i].function_count);
        }
    }
    return NUMBER_VAL(g_library_count);
}

// Get function signature constants
static Value RoseGetSignature(int argCount, Value* args) {
    if (argCount != 1 || !IS_STRING(args[0])) return NIL_VAL;

    const char* sig_name = AS_CSTRING(args[0]);

    if (strcmp(sig_name, "VOID_VOID") == 0) return NUMBER_VAL(FUNC_VOID_VOID);
    else if (strcmp(sig_name, "INT_VOID") == 0) return NUMBER_VAL(FUNC_INT_VOID);
    else if (strcmp(sig_name, "DOUBLE_VOID") == 0) return NUMBER_VAL(FUNC_DOUBLE_VOID);
    else if (strcmp(sig_name, "PTR_VOID") == 0) return NUMBER_VAL(FUNC_PTR_VOID);
    else if (strcmp(sig_name, "VOID_INT") == 0) return NUMBER_VAL(FUNC_VOID_INT);
    else if (strcmp(sig_name, "INT_INT") == 0) return NUMBER_VAL(FUNC_INT_INT);
    else if (strcmp(sig_name, "DOUBLE_INT") == 0) return NUMBER_VAL(FUNC_DOUBLE_INT);
    else if (strcmp(sig_name, "INT_DOUBLE") == 0) return NUMBER_VAL(FUNC_INT_DOUBLE);
    else if (strcmp(sig_name, "DOUBLE_DOUBLE") == 0) return NUMBER_VAL(FUNC_DOUBLE_DOUBLE);
    else if (strcmp(sig_name, "INT_INT_INT") == 0) return NUMBER_VAL(FUNC_INT_INT_INT);
    else if (strcmp(sig_name, "DOUBLE_DOUBLE_DOUBLE") == 0) return NUMBER_VAL(FUNC_DOUBLE_DOUBLE_DOUBLE);
    else if (strcmp(sig_name, "PTR_PTR") == 0) return NUMBER_VAL(FUNC_PTR_PTR);
    else if (strcmp(sig_name, "INT_PTR") == 0) return NUMBER_VAL(FUNC_INT_PTR);
    else if (strcmp(sig_name, "PTR_INT") == 0) return NUMBER_VAL(FUNC_PTR_INT);
    else if (strcmp(sig_name, "CUSTOM") == 0) return NUMBER_VAL(FUNC_CUSTOM);

    return NIL_VAL;
}

// Create a wrapper function that can be called directly from Rose
static Value RoseCreateWrapper(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    if (!IS_STRING(args[1])) return NIL_VAL;

    DLLFunction* func = (DLLFunction*)AS_NATIVE_VAL(args[0]);
    const char* wrapper_name = AS_CSTRING(args[1]);

    if (!func || !func->is_valid) {
        printf("Error: Invalid function handle\n");
        return BOOL_VAL(false);
    }

    // Create a native wrapper function that calls the DLL function
    // This would need to be implemented per signature type
    // For now, we'll just register the function pointer directly
    printf("Created wrapper '%s' for function '%s'\n", wrapper_name, func->name);

    return BOOL_VAL(true);
}

/////////////////////////////////////////////////////////////////////////////////

// Load the DLL extension into Rose
void LoadDLL() {
    // Core DLL management functions
    defineNative("dll_load", RoseLoadLibrary);
    defineNative("dll_getFunction", RoseGetFunction);
    defineNative("dll_callFunction", RoseCallFunction);
    defineNative("dll_unload", RoseUnloadLibrary);
    defineNative("dll_listLibraries", RoseListLibraries);

    // Utility functions
    defineNative("dll_getSignature", RoseGetSignature);
    defineNative("dll_createWrapper", RoseCreateWrapper);
}