#include "string.h"
#include "../../value.h"
#include "../../vm.h"
#include "../../object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// read a text file
static Value Strlen(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_STRING(args[0])) return NIL_VAL;

    const char* string = AS_CSTRING(args[0]);
    
    return NUMBER_VAL(strlen(string));
}
/////////////////////////////////////////////////////////////////////////////////

void LoadString() {
    // string functions
    defineNative("strlen", Strlen);
}