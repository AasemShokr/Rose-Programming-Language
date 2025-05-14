#include "system.h"
#include "../../value.h"
#include "../../vm.h"
#include "../../object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

// Functions
// Clear screen
static Value ClearScreen(int argCount, Value* args) {
	system("cls");
	return NIL_VAL;
}

static Value CAddress(int argCount, Value* args) {
	if (argCount != 1) return NIL_VAL;
	return NUMBER_VAL((double)((int) & args[0]));
}

static Value Type(int argCount, Value* args) {
	if (argCount != 1) return NIL_VAL;

	char* buffer = NULL;
	int length = 0;

	switch (args[0].type)
	{
	case VAL_BOOL:
		length = 8;
		buffer = malloc(length + 1);
		memcpy(buffer, "<string>", length + 1);
		break;
	case VAL_NIL:
		length = 6;
		buffer = malloc(length + 1);
		memcpy(buffer, "<none>", length + 1);
		break;
	case VAL_NUMBER:
		length = 8;
		buffer = malloc(length + 1);
		memcpy(buffer, "<number>", length + 1);
		break;
	case VAL_OBJ:
		length = 8;
		buffer = malloc(length + 1);
		memcpy(buffer, "<object>", length + 1);
		break;
	case VAL_NATIVE:
		length = 12;
		buffer = malloc(length + 1);
		memcpy(buffer, "<native value>", length + 1);
		break;
	default:
		break;
	}

	Value string = OBJ_VAL(takeString(buffer, length, true));
	return string;
}
////////////

// Memory leak : name leakes when main struct gets freed! // solved
// Mem leak: make sure native values are freed

void LoadSystem() {
	defineNative("cls", ClearScreen);
	defineNative("type", Type);
	defineNative("cp", CAddress);
}