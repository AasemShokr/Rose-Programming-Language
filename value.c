#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "value.h"
#include "object.h"

void initValueArray(ValueArray* array) {
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
  if (array->capacity < array->count + 1) {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->values = GROW_ARRAY(Value, array->values,
                               oldCapacity, array->capacity);
  }

  array->values[array->count] = value;
  array->count++;
}

void freeValueArray(ValueArray* array) {
    // delete natives first
    for (int i = 0; i < array->count; i++) {
        if (IS_NATIVE_VAL(array->values[i])) {
            free(AS_NATIVE_VAL(array->values[i]));
        }
    }
    ///////////////////////
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}

void printValue(Value value) {
    switch (value.type) {
    case VAL_BOOL:
        printf(AS_BOOL(value) ? "true" : "false");
        break;
    case VAL_NIL: printf("nil"); break;
    case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
    case VAL_NATIVE: printf("<native value>"); break;
    case VAL_OBJ: printObject(value); break;
    }
}

void printValueArray(ValueArray* array){
    printf("[Value Array, count '%i',  capacity '%i']\n[", array->count, array->capacity);
    for(int i = 0; i < array->count; i++){
        printf(" %f", AS_NUMBER(array->values[i]));
        if(i != array->count - 1)
            printf(" | ");
    }
    printf("]\n");
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:    return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:    return AS_OBJ(a) == AS_OBJ(b);
        case VAL_NATIVE: return AS_NATIVE_VAL(a) == AS_NATIVE_VAL(b);
        default:         return false; // Unreachable.
    }
}