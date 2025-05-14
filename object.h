#ifndef ROSE_OBJECT_H
#define ROSE_OBJECT_H
#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)

#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))
#define AS_INSTANCE(value)     ((ObjInstance*)AS_OBJ(value))
#define AS_CLASS(value)        ((ObjClass*)AS_OBJ(value))
#define AS_ARRAY(value)        ((ObjArray*)AS_OBJ(value))
#define AS_CLOSURE(value)      ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)       (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)

#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
#define IS_INSTANCE(value)     isObjType(value, OBJ_INSTANCE)
#define IS_CLASS(value)        isObjType(value, OBJ_CLASS)
#define IS_CLOSURE(value)      isObjType(value, OBJ_CLOSURE)
#define IS_STRING(value)       isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value)     isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value)       isObjType(value, OBJ_NATIVE)
#define IS_ARRAY(value)        isObjType(value, OBJ_ARRAY)

typedef enum {
	OBJ_STRING,
	OBJ_NATIVE,
	OBJ_ARRAY,
	OBJ_FUNCTION,
	OBJ_CLOSURE,
	OBJ_UPVALUE,
	OBJ_CLASS,
	OBJ_INSTANCE,
	OBJ_BOUND_METHOD
} ObjType;

struct Obj {
	ObjType type;
	bool isMarked;
	struct Obj* next;
};

// Arrays
struct ObjArray
{
	Obj obj;
	ObjString* name;
	int length;
	ValueArray* val_array;
};
/////////

struct ObjString {
	Obj obj;
	int length;
	char* chars;
	uint32_t hash;
};

typedef struct ObjUpvalue {
	Obj obj;
	Value* location;
	struct ObjUpvalue* next;
	Value closed;
} ObjUpvalue;

typedef struct {
	Obj obj;
	int arity;
	int upvalueCount;
	Chunk chunk;
	ObjString* name;
} ObjFunction;

typedef struct {
	Obj obj;
	ObjFunction* function;
	ObjUpvalue** upvalues;
	int upvalueCount;
} ObjClosure;

typedef struct {
	Obj obj;
	ObjString* name;
	Table methods;
} ObjClass;

typedef struct {
	Obj obj;
	ObjClass* klass;
	Table fields;
} ObjInstance;

typedef struct {
	Obj obj;
	Value receiver;
	ObjClosure* method;
} ObjBoundMethod;


ObjClosure* newClosure(ObjFunction* function);

typedef Value(*NativeFn)(int argCount, Value* args);

typedef struct {
	Obj obj;
	NativeFn function;
} ObjNative;

// strings
ObjString* takeString(char* chars, int length, bool canDelete);
ObjString* copyString(const char* chars, int length);
ObjUpvalue* newUpvalue(Value* slot);

// OOP
ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjClass* newClass(ObjString* name);
ObjInstance* newInstance(ObjClass* klass);

// functions
ObjFunction* newFunction();
ObjNative* newNative(NativeFn function);

void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
	return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif