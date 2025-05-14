#include "math.h"
#include "../../value.h"
#include "../../vm.h"
#include "../../object.h"
#include <math.h>

// Math Functions
// Acos
static Value Acos(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(acos(number));
}
// Asin
static Value Asin(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(asin(number));
}
// Atan
static Value Atan(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(atan(number));
}
// Atan2
static Value Atan2(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    if (!IS_NUMBER(args[1])) return NIL_VAL;
    double number1 = AS_NUMBER(args[0]);
    double number2 = AS_NUMBER(args[1]);
    return NUMBER_VAL(atan2(number1, number2));
}
// Sin
static Value Sin(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(sin(number));
}
// Cos
static Value Cos(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(cos(number));
}
// tan
static Value Tan(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(tan(number));
}
// Log
static Value Log(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(log(number));
}
// pow
static Value Pow(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    if (!IS_NUMBER(args[1])) return NIL_VAL;
    double number1 = AS_NUMBER(args[0]);
    double number2 = AS_NUMBER(args[1]);
    return NUMBER_VAL(pow(number1, number2));
}
// sqrt
static Value Sqrt(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(sqrt(number));
}
// ceil
static Value Ceil(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(ceil(number));
}
// floor
static Value Floor(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(floor(number));
}
// abs
static Value Abs(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NUMBER(args[0])) return NIL_VAL;
    double number = AS_NUMBER(args[0]);
    return NUMBER_VAL(abs(number));
}
/////////////////////////////////////////////////////////////////////////////////

void LoadMath() {
    defineNative("sys_lib_math_acos", Acos);
    defineNative("sys_lib_math_asin", Asin);
    defineNative("sys_lib_math_atan", Atan);
    defineNative("sys_lib_math_atan2", Atan2);
    defineNative("sys_lib_math_sin", Sin);
    defineNative("sys_lib_math_cos", Cos);
    defineNative("sys_lib_math_tan", Tan);
    defineNative("sys_lib_math_log", Log);
    defineNative("sys_lib_math_pow", Pow);
    defineNative("sys_lib_math_sqrt", Sqrt);
    defineNative("sys_lib_math_ceil", Ceil);
    defineNative("sys_lib_math_floor", Floor);
    defineNative("sys_lib_math_abs", Abs);
}