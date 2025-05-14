#include "array.h"
#include "value.h"
#include "vm.h"
#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Value ArrayGet(int argCount, Value* args) {
	if (argCount != 2) return NIL_VAL;
	ValueArray* val_array = ((ValueArray*)AS_NATIVE_VAL(args[0]));
	Value val = val_array->values[(int)AS_NUMBER(args[1])];
	return val;
}

static Value ArraySet(int argCount, Value* args) {
	if (argCount != 3) return NIL_VAL;
	ValueArray* val_array = ((ValueArray*)AS_NATIVE_VAL(args[0]));
	val_array->values[(int)AS_NUMBER(args[1])] = args[2];
	return NIL_VAL;
}

static Value ArrayLength(int argCount, Value* args) {
	if (argCount != 1) return NIL_VAL;
	ValueArray* val_array = ((ValueArray*)AS_NATIVE_VAL(args[0]));
	Value val = NUMBER_VAL(val_array->count);
	return val;
}

static Value ArrayAdd(int argCount, Value* args) {
	if (argCount != 2) return NIL_VAL;
	ValueArray* val_array = ((ValueArray*)AS_NATIVE_VAL(args[0]));
	writeValueArray(val_array, args[1]);
	return NIL_VAL;
}

void LoadArray() {
	defineNative("array_get", ArrayGet);
	defineNative("array_set", ArraySet);
	defineNative("array_len", ArrayLength);
	defineNative("array_add", ArrayAdd);
}