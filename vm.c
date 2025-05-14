#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "common.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "natives.h"

#ifdef _WIN32
#include <windows.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

VM vm;

static bool callValue(Value callee, int argCount);
static char* readFile(const char* path);

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        /*fprintf(stderr, "[line %d] in ",
            function->chunk.lines[instruction]);*/
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        }
        else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    resetStack();
}

void defineNative(const char* name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

void defineGlobalVar(const char* name, Value val) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(val);
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

void initVM() {
    resetStack();
    vm.objects = NULL;

    // gc
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;

    initTable(&vm.strings);
    initTable(&vm.globals);

    // OOP
    vm.initString = NULL;
    vm.initString = copyString("construct", 9);
    vm.destString = NULL;
    vm.destString = copyString("destruct", 8);

    // native functions
    DefineNativeFunctions();
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    vm.initString = NULL;
    vm.destString = NULL;
    freeObjects();
}

// TODO: Check for stack overflow or dynamically grow the stack.
void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure* closure, int argCount) {

    if (argCount != closure->function->arity) {
        runtimeError("Expected %d arguments but got %d.",
            closure->function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
        case OBJ_BOUND_METHOD: {
            ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
            vm.stackTop[-argCount - 1] = bound->receiver;
            return call(bound->method, argCount);
        }
        case OBJ_CLASS: {
            ObjClass* klass = AS_CLASS(callee);
            vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));

            // call init
            Value initializer;
            if (tableGet(&klass->methods, vm.initString, &initializer)) {
                return call(AS_CLOSURE(initializer), argCount);
            }
            else if (argCount != 0) {
                runtimeError("Expected 0 arguments but got %d.", argCount);
                return false;
            }
            return true;
        }
        case OBJ_CLOSURE:
            return call(AS_CLOSURE(callee), argCount);
        case OBJ_NATIVE: {
            NativeFn native = AS_NATIVE(callee);
            Value result = native(argCount, vm.stackTop - argCount);
            vm.stackTop -= argCount + 1;
            push(result);
            return true;
        }
        default:
            break; // Non-callable object type.
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}

static bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }
    return call(AS_CLOSURE(method), argCount);
}

static bool invoke(ObjString* name, int argCount) {
    Value receiver = peek(argCount); // peek at argc to skip them to instance

    if (!IS_INSTANCE(receiver)) {
        runtimeError("Only instances have methods.");
        return false;
    }

    ObjInstance* instance = AS_INSTANCE(receiver);

    Value value;
    if (tableGet(&instance->fields, name, &value)) {
        vm.stackTop[-argCount - 1] = value;
        return callValue(value, argCount);
    }

    return invokeFromClass(instance->klass, name, argCount);
}

static bool bindMethod(ObjClass* klass, ObjString* name) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(peek(0),
        AS_CLOSURE(method));
    pop();
    push(OBJ_VAL(bound));
    return true;
}

static ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = newUpvalue(local);

    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    }
    else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    while (vm.openUpvalues != NULL &&
        vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

static void defineMethod(ObjString* name) {
    Value method = peek(0);
    ObjClass* klass = AS_CLASS(peek(1));
    tableSet(&klass->methods, name, method);
    pop();
}

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    // peaking to protect from garbage collection
    ObjString* b = AS_STRING(peek(0));
    ObjString* a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length, true);
    pop();
    pop();
    push(OBJ_VAL(result));
}

int ReadInt(CallFrame* frame) {
    int constant = 0;
    int pos = frame->ip - frame->closure->function->chunk.code;

    constant |= ((int)(frame->closure->function->chunk.code[pos++]));
    constant |= ((int)(frame->closure->function->chunk.code[pos++])) << 8;
    constant |= ((int)(frame->closure->function->chunk.code[pos++])) << 16;
    constant |= ((int)(frame->closure->function->chunk.code[pos++])) << 24;
    
    return constant;
}

Value ReadConstant(CallFrame* frame) {
    int index = ReadInt(frame);
    Value v = frame->closure->function->chunk.constants.values[index];
    (frame->ip) += 4;
    return v;
}

CallFrame* frame;

static bool callDestructor(ObjInstance* instance) {
    ObjClass* klass = instance->klass;
    Value destructor;

    if (tableGet(&klass->methods, vm.destString, &destructor)) {
        push(OBJ_VAL(instance));

        ObjClosure* closure = AS_CLOSURE(destructor);
        if (!call(closure, 0)) {
            // Handle error if call fails
            runtimeError("Failed to call destructor for instance of '%s'.", klass->name->chars);
            return false;
        }
        pop(); // Remove the instance from the stack
    }

    return true;
}

static InterpretResult run() {

    frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() (ReadConstant(frame))
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_SHORT() \
    (frame->ip += 2, \
    (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstruction(&frame->closure->function->chunk,
            (int)(frame->ip - frame->closure->function->chunk.code));
#endif
        uint8_t instruction = READ_BYTE();
        switch (instruction) {
            case OP_CONSTANT_LONG: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            // Arethmetic operations 
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_NIL: push(NIL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_ADD: {
                Value a = peek(0);
                Value b = peek(1);
                if (IS_STRING(a) && IS_STRING(b)) {
                    concatenate();
                }
                else if (IS_NUMBER(a) && IS_NUMBER(b)) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                }
                else {
                    runtimeError(
                        "Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, / ); break;
            case OP_GREATER:  BINARY_OP(BOOL_VAL, > ); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, < ); break;
            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            // Print
            case OP_PRINT: { // TODO: add support for println and print
                printValue(pop());
                break;
            }
            case OP_POP: pop(); break;
            // GLobal variables
            case OP_DEFINE_GLOBAL: { // Set
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }
            case OP_GET_GLOBAL: { // Get
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined global variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Undefined global variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_LOCAL: {
                int slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                int slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_ARRAY: {
                Value array_count = pop();
                ValueArray* array_entries = (ValueArray*)malloc(sizeof(ValueArray));
                initValueArray(array_entries);
                for (int i = 0; i < (int)array_count.as.number; i++) {
                    Value val = pop();
                    writeValueArray(array_entries, val);
                }
                push(NATIVE_VAL(array_entries, sizeof(ValueArray)));
                break;
            }
            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(function);
                push(OBJ_VAL(closure));
                for (int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] =
                            captureUpvalue(frame->slots + index);
                    }
                    else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_CLOSE_UPVALUE:
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;
            case OP_CLASS:
                push(OBJ_VAL(newClass(READ_STRING())));
                break;
            case OP_GET_PROPERTY: {
                if (!IS_INSTANCE(peek(0))) {
                    runtimeError("Only instances have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(peek(0));
                ObjString* name = READ_STRING();

                Value value;
                if (tableGet(&instance->fields, name, &value)) {
                    pop(); // Instance.
                    push(value);
                    break;
                }

                if (!bindMethod(instance->klass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(1))) {
                    runtimeError("Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(peek(1));
                tableSet(&instance->fields, READ_STRING(), peek(0));
                Value value = pop();
                pop();
                push(value);
                break;
            }
            case OP_METHOD: {
                defineMethod(READ_STRING());
                break;
            }
            case OP_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                if (!invoke(method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_INHERIT: {
                Value superclass = peek(1);

                if (!IS_CLASS(superclass)) {
                    runtimeError("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjClass* subclass = AS_CLASS(peek(0));
                tableAddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
                pop(); // Subclass.
                break;
            }
            case OP_GET_SUPER: {
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop());

                if (!bindMethod(superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUPER_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                ObjClass* superclass = AS_CLASS(pop());
                if (!invokeFromClass(superclass, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            // Return
            case OP_RETURN: {
                Value result = pop();
                closeUpvalues(frame->slots);
                vm.frameCount--;

                if (vm.frameCount == 0) {
                    pop();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_INCLUDE: {
                Value exp = pop();

                // check if importing a string
                if (!IS_STRING(exp)) {
                    runtimeError("Package name can only be a string.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjString* package = AS_STRING(exp);

                bool isPackage = AS_BOOL(frame->closure->function->chunk.constants.values[0]);
                Value ExePath = frame->closure->function->chunk.constants.values[1];
                Value DirPath = frame->closure->function->chunk.constants.values[2];
                
                char* fileName = package->chars;
                char path[_MAX_PATH] = "";
                char exe_path[_MAX_PATH] = "";
                char dir_path[_MAX_PATH] = "";

                strcat(exe_path, AS_STRING(ExePath)->chars);
                strcat(dir_path, AS_STRING(DirPath)->chars);
                strcat(path, AS_STRING(DirPath)->chars);
                strcat(path, "\\");
                strcat(path, fileName);

                // pass the file directory not package directory 'to enable recursion'
                char rel_dir[FILENAME_MAX + 1];
                rel_dir[0] = '\0';

                char* ptr = strrchr(fileName, '\\');
                if (ptr != NULL) {
                    int last_index = ptr - fileName;
                    for (int i = 0; i < last_index; i++) {
                        rel_dir[i] = fileName[i];
                    }
                    rel_dir[last_index] = '\0';
                
                    int temp = strlen(dir_path);
                    dir_path[temp] = '\\';
                    dir_path[temp + 1] = '\0';

                    int dirlen = strlen(dir_path);
                    int rellen = strlen(rel_dir);
                    for (int i = dirlen; i < dirlen + rellen; i++) {
                        dir_path[i] = rel_dir[i - dirlen];
                    }
                    //strcat(dir_path, rel_dir);
                }

                // read file
                const char* file = readFile(path);

                // Call the document
                // pass directory to compile
                ObjFunction* function = compile(file, exe_path, dir_path, isPackage);
                if (function == NULL) return INTERPRET_COMPILE_ERROR;

                push(OBJ_VAL(function));
                ObjClosure* closure = newClosure(function);
                pop();
                
                if (!callValue(OBJ_VAL(closure), 0)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];

                break;
            }
            case OP_IMPORT: {
                Value exp = pop();

                // check if importing a string
                if (!IS_STRING(exp)) {
                    runtimeError("Package name can only be a string.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjString* package = AS_STRING(exp);

                ///Main package file __MAIN__
                Value ExePath = frame->closure->function->chunk.constants.values[1];
                Value DirPath = frame->closure->function->chunk.constants.values[2];
                char* pakcage_name = package->chars;

                char packagePath[_MAX_PATH] = "";
                char packageMain[_MAX_PATH] = "";

                strcat(packagePath, AS_CSTRING(ExePath));
                strcat(packagePath, "\\packages\\");
                strcat(packagePath, package->chars);
                strcat(packageMain, packagePath);
                strcat(packageMain, "\\__MAIN__.rose");

                // read actual package
                const char* file = readFile(packageMain);

                // Call the document
                ObjFunction* function = compile(file, AS_CSTRING(ExePath), packagePath, true);
                if (function == NULL) return INTERPRET_COMPILE_ERROR;

                push(OBJ_VAL(function));
                ObjClosure* closure = newClosure(function);
                pop();

                if (!callValue(OBJ_VAL(closure), 0)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];

                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
    char buff[FILENAME_MAX];
    char curDir[FILENAME_MAX];

    GetCurrentDir(curDir, FILENAME_MAX);

    // Windows only
    GetModuleFileNameA(GetModuleHandle(0), buff, sizeof(buff));

    char* ptr = strrchr(buff, '\\');

    char dir[FILENAME_MAX + 1];
    int last_index = ptr - buff;
    
    //memcpy(dir, buff, last_index);
    for (int i = 0; i < last_index; i++) {
        dir[i] = buff[i];
    }
    dir[last_index] = '\0';

    ObjFunction* function = compile(source, dir, curDir, false);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0);

    return run();
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