#include "sdl.h"
#include "../../value.h"
#include "../../vm.h"
#include "../../object.h"
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>

// SDL Functions
// Create Window
static Value CreateWindow(int argCount, Value* args) {
    if (argCount != 5) return NIL_VAL;
    if (!IS_STRING(args[0])) return NIL_VAL;
    if (!IS_NUMBER(args[1])) return NIL_VAL;
    if (!IS_NUMBER(args[2])) return NIL_VAL;
    if (!IS_NUMBER(args[3])) return NIL_VAL;
    if (!IS_NUMBER(args[4])) return NIL_VAL;

    const char* title = AS_CSTRING(args[0]);
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    double w = AS_NUMBER(args[3]);
    double h = AS_NUMBER(args[4]);
    
    SDL_Window* window = SDL_CreateWindow(title, // creates a window
        (int) x,
        (int) y,
        (int) w,
        (int) h, 0);
    return NATIVE_VAL(window, sizeof(SDL_Window*));
}
// INIT
static Value SDL_INIT(int argCount, Value* args) {
    int init = SDL_Init(SDL_INIT_EVERYTHING);
    return NUMBER_VAL(init);
}
// Ger Error
static Value rSDL_Error(int argCount, Value* args) {
    const char* error = SDL_GetError();
    return OBJ_VAL(copyString(error, SDL_strlen(error)));
}
static Value SDL_Window_Centered(int argCount, Value* args) {
    return NUMBER_VAL(SDL_WINDOWPOS_CENTERED);
}
static Value rSDL_QUIT(int argCount, Value* args) {
    SDL_Quit();
    return NIL_VAL;
}
int ev = 0; // Convert to SDL_EVENT
static Value SDL_POLLEVENT(int argCount, Value* args) {
    ev = SDL_PollEvent(&ev);
    return NUMBER_VAL((double)ev);
}
static Value SDL_CREATERENDERER(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_NATIVE_VAL(args[0])) return NIL_VAL;

    SDL_Renderer* renderer = SDL_CreateRenderer(AS_NATIVE_VAL(args[0]), -1, 0);
    return NATIVE_VAL(renderer, sizeof(SDL_Renderer*));
}
static Value SDL_LOADIMAGE(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    if (!IS_STRING(args[0])) return NIL_VAL;

    const char* path = AS_CSTRING(args[0]);

    SDL_Surface* image = SDL_LoadBMP(path);
    return NATIVE_VAL(image, sizeof(SDL_Surface*));
}
static Value SDL_RENDERPRESENT(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    SDL_Renderer* renderer = AS_NATIVE_VAL(args[0]);
    SDL_RenderPresent(renderer);
    return NIL_VAL;
}
static Value SDL_CREATETEXTURE(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;

    SDL_Renderer* renderer = AS_NATIVE_VAL(args[0]);
    SDL_Surface* image = AS_NATIVE_VAL(args[1]);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
    return NATIVE_VAL(texture, sizeof(SDL_Texture*));
}
static Value SDL_RENDERCOPY(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;

    SDL_Renderer* renderer = AS_NATIVE_VAL(args[0]);
    SDL_Texture* texture = AS_NATIVE_VAL(args[1]);

    SDL_RenderCopy(renderer, texture, NULL, NULL);
    return NIL_VAL;
}
/////////////////////////////////////////////////////////////////////////////////

void LoadSDL() {
    defineNative("sys_lib_sdl_init", SDL_INIT);
    defineNative("sys_lib_sdl_get_error", rSDL_Error);
    defineNative("sys_lib_sdl_cerateWindow", CreateWindow);
    defineNative("sys_lib_sdl_window_centered", SDL_Window_Centered);
    defineNative("sys_lib_sdl_quit", rSDL_QUIT);
    defineNative("sys_lib_sdl_poll_event", SDL_POLLEVENT);
    defineNative("sys_lib_sdl_create_renderer", SDL_CREATERENDERER);
    defineNative("sys_lib_sdl_load_image", SDL_LOADIMAGE);
    defineNative("sys_lib_sdl_create_texture", SDL_CREATETEXTURE);
    defineNative("sys_lib_sdl_render_copy", SDL_RENDERCOPY);
    defineNative("sys_lib_sdl_render_present", SDL_RENDERPRESENT);
}