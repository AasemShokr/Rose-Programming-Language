#include "sdl.h"
#include "../../value.h"
#include "../../vm.h"
#include "../../object.h"
#include <math.h>
#include <SFML/Audio.h>
#include <SFML/Graphics.h>
#include <stdio.h>

// SDL Functions
// Create Window
static Value SFCreateWindow(int argCount, Value* args) {
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
    
    sfVideoMode mode = { w, h, 32 };
    sfRenderWindow* window = sfRenderWindow_create(mode, title, sfResize | sfClose, NULL);
    sfRenderWindow_setVerticalSyncEnabled(window, false);

    return NATIVE_VAL(window, sizeof(sfRenderWindow*));
}

static Value SFIsWindowOpen(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    return BOOL_VAL(sfRenderWindow_isOpen(AS_NATIVE_VAL(args[0])));
}

static Value SFUpdateWindowEvents(int argCount, Value* args) {
    sfEvent ev;
    while (sfRenderWindow_pollEvent(AS_NATIVE_VAL(args[0]), &ev))
    {
        /* Close window : exit */
        if (ev.type == sfEvtClosed)
            sfRenderWindow_close(AS_NATIVE_VAL(args[0]));
    }

    return BOOL_VAL(true);
}

static Value SFUpdateWindow(int argCount, Value* args) {
    sfRenderWindow_display(AS_NATIVE_VAL(args[0]));
    return BOOL_VAL(true);
}

static Value SFLoadTexture(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    const char* path = AS_CSTRING(args[0]);
    sfTexture* texture = sfTexture_createFromFile(path, NULL);
    return NATIVE_VAL(texture, sizeof(sfTexture*));
}

static Value SFLoadFont(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    const char* path = AS_CSTRING(args[0]);
    sfFont* texture = sfFont_createFromFile(path);
    return NATIVE_VAL(texture, sizeof(sfFont*));
}

static Value SFDrawText(int argCount, Value* args) {
    if (argCount != 10) return NIL_VAL;
    const void* path;
    if (IS_NUMBER(args[0])) {
        path = malloc(50);
        double num = AS_NUMBER(args[0]);
        char output[50];
        snprintf(path, 50, "%f", num);
    }
    else {
        path = AS_CSTRING(args[0]);
    }
    sfFont* font = AS_NATIVE_VAL(args[1]);
    // color
    double r = AS_NUMBER(args[2]);
    double g = AS_NUMBER(args[3]);
    double b = AS_NUMBER(args[4]);
    double a = AS_NUMBER(args[5]);
    double size = AS_NUMBER(args[6]);
    double x = AS_NUMBER(args[7]);
    double y = AS_NUMBER(args[8]);
    sfText* text = sfText_create();
    sfText_setString(text, path);
    sfText_setFont(text, font);
    sfText_setCharacterSize(text, size);
    sfText_setColor(text, sfColor_fromRGBA((sfUint8)r, (sfUint8)g, (sfUint8)b, (sfUint8)a));
    sfText_setPosition(text, (sfVector2f) { x, y });
    sfRenderWindow_drawText(AS_NATIVE_VAL(args[9]), text, NULL);
    sfText_destroy(text);
    if(IS_NUMBER(args[0])) free(path); // solve mem leak
    return BOOL_VAL(true);
}

static Value SFCreateSprite(int argCount, Value* args) {
    return NATIVE_VAL(sfSprite_create(), sizeof(sfSprite*));
}

static Value SFTextureDestroy(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    sfTexture_destroy(AS_NATIVE_VAL(args[0]));
    return BOOL_VAL(true);
}

static Value SFSpriteDestroy(int argCount, Value* args) {
    if (argCount != 1) return NIL_VAL;
    sfSprite_destroy(AS_NATIVE_VAL(args[0]));
    return BOOL_VAL(true);
}

static Value SFSpriteSetTexture(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    // 1- sprite
    // 2- texture
    sfSprite_setTexture(AS_NATIVE_VAL(args[0]), AS_NATIVE_VAL(args[1]), sfTrue);
    return BOOL_VAL(true);
}

static Value SFDrawSprite(int argCount, Value* args) {
    if (argCount != 4) return NIL_VAL;
    // 1- window
    // 2- sprite
    // 3- x
    // 4- y
    double x = AS_NUMBER(args[2]);
    double y = AS_NUMBER(args[3]);
    sfSprite_setPosition(AS_NATIVE_VAL(args[1]), (sfVector2f) { x, y });
    sfRenderWindow_drawSprite(AS_NATIVE_VAL(args[0]), AS_NATIVE_VAL(args[1]), NULL);
    return BOOL_VAL(true);
}

static Value SFScaleSprite(int argCount, Value* args) {
    if (argCount != 3) return NIL_VAL;
    // 1- window
    // 2- sprite
    // 3- x
    // 4- y
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    sfSprite_setScale(AS_NATIVE_VAL(args[0]), (sfVector2f) { x, y });
    return BOOL_VAL(true);
}

static Value SFSpriteSetRect(int argCount, Value* args) {
    if (argCount != 5) return NIL_VAL;
    // 1- window
    // 2- sprite
    // 3- x
    // 4- y
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    double w = AS_NUMBER(args[3]);
    double h = AS_NUMBER(args[4]);
    sfSprite_setTextureRect(AS_NATIVE_VAL(args[0]), (sfIntRect) {x, y, w, h});
    return BOOL_VAL(true);
}

static Value SFRotateSprite(int argCount, Value* args) {
    if (argCount != 2) return NIL_VAL;
    // 1- window
    // 2- sprite
    // 3- x
    // 4- y
    double x = AS_NUMBER(args[1]);
    sfSprite_setRotation(AS_NATIVE_VAL(args[0]), (float)x);
    return NIL_VAL;
}

static Value SFTextureGetWidth(int argCount, Value* args) {
    // 1- texture
    double w = (sfTexture_getSize(AS_NATIVE_VAL(args[0])).x);
    return NUMBER_VAL(w);
}

static Value SFTextureGetHeight(int argCount, Value* args) {
    // 1- texture
    double h = (sfTexture_getSize(AS_NATIVE_VAL(args[0])).y);
    return NUMBER_VAL(h);
}

static Value SFClear(int argCount, Value* args) {
    // 0- window
    double r = AS_NUMBER(args[1]);
    double g = AS_NUMBER(args[2]);
    double b = AS_NUMBER(args[3]);
    double a = AS_NUMBER(args[4]);
    sfRenderWindow_clear(AS_NATIVE_VAL(args[0]),
        (sfColor) { (sfUint8)r, (sfUint8)g, (sfUint8)b, (sfUint8)a });
    return NIL_VAL;
}

// Draw Shapes
static Value SFDrawLine(int argCount, Value* args) {
    if (argCount != 9) return NIL_VAL;
    // 0- window
    // 1- x1
    // 2- y1
    // 3- x2
    // 4- y2
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    double x2 = AS_NUMBER(args[3]);
    double y2 = AS_NUMBER(args[4]);
    // color
    double r = AS_NUMBER(args[5]);
    double g = AS_NUMBER(args[6]);
    double b = AS_NUMBER(args[7]);
    double a = AS_NUMBER(args[8]);

    sfVertex line[2];
    line[0].position.x = x;
    line[0].position.y = y;
    line[0].color.r = (sfUint8)r;
    line[0].color.g = (sfUint8)g;
    line[0].color.b = (sfUint8)b;
    line[0].color.a = (sfUint8)a;

    line[1].position.x = x2;
    line[1].position.y = y2;
    line[1].color.r = (sfUint8)r;
    line[1].color.g = (sfUint8)g;
    line[1].color.b = (sfUint8)b;
    line[1].color.a = (sfUint8)a;
    
    sfRenderWindow_drawPrimitives(AS_NATIVE_VAL(args[0]), line, 2, sfLines, NULL);
    return BOOL_VAL(true);
}

static Value SFDrawCircle(int argCount, Value* args) {
    if (argCount != 8) return NIL_VAL;
    // 0- window
    // 1- x1
    // 2- y1
    // 3- x2
    // 4- y2
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    double radius = AS_NUMBER(args[3]);
    // color
    double r = AS_NUMBER(args[4]);
    double g = AS_NUMBER(args[5]);
    double b = AS_NUMBER(args[6]);
    double a = AS_NUMBER(args[7]);

    sfCircleShape* circle = sfCircleShape_create();
    sfCircleShape_setPosition(circle, (sfVector2f) { x, y });
    sfCircleShape_setRadius(circle, radius);
    sfCircleShape_setFillColor(circle, (sfColor) { (sfUint8)r, (sfUint8)g, (sfUint8)b, (sfUint8)a });
    sfRenderWindow_drawCircleShape(AS_NATIVE_VAL(args[0]), circle, NULL);
    sfCircleShape_destroy(circle);
    return BOOL_VAL(true);
}

static Value SFDrawRect(int argCount, Value* args) {
    if (argCount != 9) return NIL_VAL;
    // 0- window
    // 1- x1
    // 2- y1
    // 3- x2
    // 4- y2
    double x = AS_NUMBER(args[1]);
    double y = AS_NUMBER(args[2]);
    double w = AS_NUMBER(args[3]);
    double h = AS_NUMBER(args[4]);
    // color
    double r = AS_NUMBER(args[5]);
    double g = AS_NUMBER(args[6]);
    double b = AS_NUMBER(args[7]);
    double a = AS_NUMBER(args[8]);

    sfRectangleShape* circle = sfRectangleShape_create();
    sfRectangleShape_setPosition(circle, (sfVector2f) { x, y });
    sfRectangleShape_setSize(circle, (sfVector2f) { w, h });
    sfRectangleShape_setFillColor(circle, (sfColor) { (sfUint8)r, (sfUint8)g, (sfUint8)b, (sfUint8)a });
    sfRenderWindow_drawRectangleShape(AS_NATIVE_VAL(args[0]), circle, NULL);
    sfRectangleShape_destroy(circle);
    return BOOL_VAL(true);
}

// Input

static Value SFGetMouseX(int argCount, Value* args) {
    // 1- texture
    double w = (sfMouse_getPosition(AS_NATIVE_VAL(args[0])).x);
    return NUMBER_VAL(w);
}

static Value SFClockCreate(int argCount, Value* args) {
    // 1- texture
    return NATIVE_VAL(sfClock_create(), sizeof(sfClock*));
}

static Value SFClockRestart(int argCount, Value* args) {
    sfClock_restart(AS_NATIVE_VAL(args[0]));
    return NIL_VAL;
}

static Value SFClockGetElapsedTime(int argCount, Value* args) {
    sfTime v = (sfClock_getElapsedTime((AS_NATIVE_VAL(args[0]))));
    return NUMBER_VAL(sfTime_asMilliseconds(v));
}

static Value SFGetMouseY(int argCount, Value* args) {
    // 1- texture
    double w = (sfMouse_getPosition(AS_NATIVE_VAL(args[0])).y);
    return NUMBER_VAL(w);
}

static Value SFIsKeyPressed(int argCount, Value* args) {
    bool w = sfKeyboard_isKeyPressed((sfKeyCode)(AS_NUMBER(args[0])));
    return BOOL_VAL(w);
}

static Value SFIsMouseButtonPressed(int argCount, Value* args) {
    bool w = sfMouse_isButtonPressed((sfMouseButton)(AS_NUMBER(args[0])));
    return BOOL_VAL(w);
}

/////////////////////////////////////////////////////////////////////////////////

void LoadSFML() {
    // window
    defineNative("sys_lib_sfml_createWindow", SFCreateWindow);
    defineNative("sys_lib_sfml_isWindowOpen", SFIsWindowOpen);
    defineNative("sys_lib_sfml_updateWindow", SFUpdateWindow);
    defineNative("sys_lib_sfml_updateWindowEvents", SFUpdateWindowEvents);
    defineNative("sys_lib_sfml_windowClear", SFClear);
    // graphics
    defineNative("sys_lib_sfml_loadTexture", SFLoadTexture);
    defineNative("sys_lib_sfml_createSprite", SFCreateSprite);
    defineNative("sys_lib_sfml_spriteSetTexture", SFSpriteSetTexture);
    defineNative("sys_lib_sfml_destroyTexture", SFTextureDestroy);
    defineNative("sys_lib_sfml_destroySprite", SFSpriteDestroy);
    defineNative("sys_lib_sfml_drawSprite", SFDrawSprite);
    defineNative("sys_lib_sfml_scaleSprite", SFScaleSprite);
    defineNative("sys_lib_sfml_spriteSetRect", SFSpriteSetRect);
    defineNative("sys_lib_sfml_rotateSprite", SFRotateSprite);
    defineNative("sys_lib_sfml_getTextureWidth", SFTextureGetWidth);
    defineNative("sys_lib_sfml_getTextureHeight", SFTextureGetHeight);
    defineNative("sys_lib_sfml_drawLine", SFDrawLine);
    // TEXT
    defineNative("sys_lib_sfml_loadFont", SFLoadFont);
    defineNative("sys_lib_sfml_drawText", SFDrawText);
    // TODO: create drawLineStream
    defineNative("sys_lib_sfml_drawCircle", SFDrawCircle);
    defineNative("sys_lib_sfml_drawRectangle", SFDrawRect);
    // input
    defineNative("sys_lib_sfml_getMouseX", SFGetMouseX);
    defineNative("sys_lib_sfml_getMouseY", SFGetMouseY);
    defineNative("sys_lib_sfml_isKeyPressed", SFIsKeyPressed);
    defineNative("sys_lib_sfml_isMouseButtonPressed", SFIsMouseButtonPressed);
    // time
    defineNative("sys_lib_sfml_clockCreate", SFClockCreate);
    defineNative("sys_lib_sfml_clockRestart", SFClockRestart);
    defineNative("sys_lib_sfml_clockGetElapsedTime", SFClockGetElapsedTime);
}