#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "natives.h"
#include "vm.h"
#include "object.h"
#include "library_runtime.h"
#include "libraries/io/io.h"
#include "libraries/system/system.h"
#include "libraries/math/math.h"
#include "libraries/string/string.h"
#include "libraries/sdl/sdl.h"
#include "libraries/sfml/sfml.h"
#include "array.h"


// Load Libraries
void DefineNativeFunctions() {
	// System
	LoadSystem();
	// IO
	LoadIO();
	// Math
	LoadMath();
	// String
	LoadString();
	// SDL
	//LoadSDL();
	//SFML
	//LoadSFML();
	//LoadArrays();
	//LoadTables();
	LoadArray();
	LoadDLL();
}