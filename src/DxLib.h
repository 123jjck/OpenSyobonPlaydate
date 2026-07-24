#pragma once

#include "pd_api.h"

#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct PDString {
    char value[192];

    PDString() = default;

    PDString(const char* text)
    {
        snprintf(value, sizeof(value), "%s", text == nullptr ? "" : text);
    }

    PDString& operator=(const char* text)
    {
        snprintf(value, sizeof(value), "%s", text == nullptr ? "" : text);
        return *this;
    }

    const char* c_str() const { return value; }
};

typedef PDString string;

#define TRUE 1
#define FALSE 0
#define byte unsigned char

extern PlaydateAPI* pd;

struct SDL_Surface {
    LCDBitmap* bitmap;
    int w;
    int h;
};

struct Mix_Chunk {
    AudioSample* sample;
    SamplePlayer* player;
};

struct Mix_Music {
    FilePlayer* player;
};

struct SDL_Joystick {};
struct TTF_Font {};

typedef uint32_t Uint32;

extern SDL_Surface* screen;
extern SDL_Joystick* joystick;
extern TTF_Font* font[64];
extern bool sound;

void PDSetDrawOffset(int x, int y);
void PDSetDrawingEnabled(bool enabled);
bool PDIsDrawingEnabled(void);
void PDFillHatchedRect(int x, int y, int width, int height, int density = 2);
float PDGetCrankChange(void);
byte ButtonJustPressed(int key);
void RequestQuitToTitle(void);

#define ScreenFlip() ((void)0)
#define GetNowCount() (pd->system->getCurrentTimeMilliseconds())
#define SDL_Delay(ms) ((void)(ms))

#define SetFontThickness(f) ((void)0)
#define StopSoundMem(s) StopSoundMemZ(s)
#define ChangeVolumeSoundMem(s, v) ((void)0)

#define DX_PLAYTYPE_LOOP -1
#define DX_PLAYTYPE_BACK 0
#define MIX_MAX_VOLUME 128
#define CheckSoundMem(soundEffect) (!(soundEffect))

void PlaySoundMem(Mix_Chunk* soundEffect, int loopMode);
Mix_Chunk* LoadSoundMem(const char* filename);
Mix_Music* LoadMusicMem(const char* filename);
void StopSoundMemZ(Mix_Chunk* soundEffect);
void Mix_HaltChannel(int channel);
void Mix_HaltMusic(void);
int Mix_PlayMusic(Mix_Music* music, int loops);
void Mix_VolumeMusic(int volume);
void Mix_FreeMusic(Mix_Music* music);
void Mix_FreeChunk(Mix_Chunk* chunk);

int DxLib_Init(void);

#define FONT_MAX 64
#define DX_FONTTYPE_NORMAL 0
#define DX_FONTTYPE_EDGE 1

void SetFontSize(byte size);
void ChangeFontType(byte type);
int PDGetTextWidth(const char* text);
void DrawString(int x, int y, const char* text, Uint32 color);
void DrawFormatString(int x, int y, Uint32 color, const char* format, ...);

enum {
    KEY_INPUT_ESCAPE = 1,
    KEY_INPUT_LEFT,
    KEY_INPUT_RIGHT,
    KEY_INPUT_DOWN,
    KEY_INPUT_UP,
    KEY_INPUT_F1,
    KEY_INPUT_O,
    KEY_INPUT_Z,
    KEY_INPUT_RETURN,
    KEY_INPUT_SPACE,
    KEY_INPUT_1,
    KEY_INPUT_2,
    KEY_INPUT_3,
    KEY_INPUT_4,
    KEY_INPUT_5,
    KEY_INPUT_6,
    KEY_INPUT_7,
    KEY_INPUT_8,
    KEY_INPUT_9,
    KEY_INPUT_0
};

#define JOYSTICK_JUMP 1

void UpdateKeys(void);
byte ProcessMessage(void);
byte CheckHitKey(int key);
byte WaitKey(void);
int SDL_JoystickGetButton(SDL_Joystick* joystick, int button);

Uint32 GetColor(byte red, byte green, byte blue);

#define DrawGraph(a, b, bitmap, transparent) DrawGraphZ(a, b, bitmap)
void DrawGraphZ(int x, int y, SDL_Surface* bitmap);

#define DrawTurnGraph(a, b, bitmap, transparent) DrawTurnGraphZ(a, b, bitmap)
void DrawTurnGraphZ(int x, int y, SDL_Surface* bitmap);
void DrawVertTurnGraph(int centerX, int centerY, SDL_Surface* bitmap);

SDL_Surface* DerivationGraph(
    int sourceX,
    int sourceY,
    int width,
    int height,
    SDL_Surface* source
);
SDL_Surface* LoadGraph(const char* filename);

void SDL_FreeSurface(SDL_Surface* surface);
void SDL_Flip(SDL_Surface* ignored);
void SDL_JoystickClose(SDL_Joystick* ignored);
void TTF_CloseFont(TTF_Font* ignored);
void IMG_Quit(void);
void TTF_Quit(void);
void Mix_Quit(void);
void SDL_Quit(void);

void pixelColor(SDL_Surface* ignored, int x, int y, Uint32 rgba);
void lineColor(
    SDL_Surface* ignored,
    int x1,
    int y1,
    int x2,
    int y2,
    Uint32 rgba
);
void rectangleColor(
    SDL_Surface* ignored,
    int x1,
    int y1,
    int x2,
    int y2,
    Uint32 rgba
);
void boxColor(
    SDL_Surface* ignored,
    int x1,
    int y1,
    int x2,
    int y2,
    Uint32 rgba
);
void ellipseColor(
    SDL_Surface* ignored,
    int centerX,
    int centerY,
    int radiusX,
    int radiusY,
    Uint32 rgba
);
void filledEllipseColor(
    SDL_Surface* ignored,
    int centerX,
    int centerY,
    int radiusX,
    int radiusY,
    Uint32 rgba
);
void SDL_FillRect(SDL_Surface* ignored, void* rect, Uint32 rgb);
