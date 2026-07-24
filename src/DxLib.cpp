#include "DxLib.h"

PlaydateAPI* pd = nullptr;
SDL_Surface* screen = nullptr;
SDL_Joystick* joystick = nullptr;
TTF_Font* font[FONT_MAX] = {};
bool sound = true;

static PDButtons heldButtons = (PDButtons)0;
static PDButtons pushedButtons = (PDButtons)0;
static byte fontSize = 16;
static byte fontType = DX_FONTTYPE_NORMAL;
static LCDFont* smallFont = nullptr;
static LCDFont* largeFont = nullptr;
static Mix_Music* currentMusic = nullptr;
static int drawOffsetX = 0;
static int drawOffsetY = 0;
static float crankChange = 0.0f;
static bool drawingEnabled = true;

static LCDColor packedRgbToColor(Uint32 rgb)
{
    const int red = (rgb >> 16) & 0xff;
    const int green = (rgb >> 8) & 0xff;
    const int blue = rgb & 0xff;
    const int luminance = red * 299 + green * 587 + blue * 114;
    return luminance >= 145000 ? kColorWhite : kColorBlack;
}

static LCDColor packedRgbaToColor(Uint32 rgba)
{
    const Uint32 rgb = ((rgba >> 8) & 0x00ffffff);
    return packedRgbToColor(rgb);
}

static void assetPathWithoutExtension(
    const char* filename,
    char* output,
    size_t outputSize
)
{
    snprintf(output, outputSize, "%s", filename);
    char* dot = strrchr(output, '.');
    if (dot != nullptr) {
        *dot = '\0';
    }
}

int DxLib_Init(void)
{
    const char* error = nullptr;
    smallFont = pd->graphics->loadFont(
        "/System/Fonts/Roobert-11-Medium-extended.pft",
        &error
    );
    if (smallFont == nullptr) {
        pd->system->error("Unable to load small system font: %s", error);
        return -1;
    }

    largeFont = pd->graphics->loadFont(
        "/System/Fonts/Roobert-20-Medium-extended.pft",
        &error
    );
    if (largeFont == nullptr) {
        pd->system->error("Unable to load large system font: %s", error);
        return -1;
    }

    pd->graphics->setFont(smallFont);
    return 0;
}

void PDSetDrawOffset(int x, int y)
{
    drawOffsetX = x;
    drawOffsetY = y;
    pd->graphics->setDrawOffset(x, y);
}

void PDSetDrawingEnabled(bool enabled)
{
    drawingEnabled = enabled;
}

bool PDIsDrawingEnabled(void)
{
    return drawingEnabled;
}

void PDFillHatchedRect(int x, int y, int width, int height, int density)
{
    if (!drawingEnabled || width <= 0 || height <= 0) {
        return;
    }

    if (density < 1)
        density = 1;
    if (density > 4)
        density = 4;

    // Diagonal ink, phase-locked to this rectangle so the texture moves with
    // world geometry instead of swimming as the camera scrolls.
    static const uint8_t blackPhases[4][4] = {
        {0, 8, 8, 8},
        {0, 4, 8, 8},
        {0, 3, 6, 8},
        {0, 1, 4, 5}
    };
    LCDPattern pattern;
    const int screenOriginX = ((x + drawOffsetX) % 8 + 8) % 8;
    const int screenOriginY = ((y + drawOffsetY) % 8 + 8) % 8;

    for (int screenY = 0; screenY < 8; screenY++) {
        const int localY = (screenY - screenOriginY + 8) % 8;
        uint8_t row = 0;
        for (int screenX = 0; screenX < 8; screenX++) {
            const int localX = (screenX - screenOriginX + 8) % 8;
            const int phase = (localX + localY) % 8;
            bool black = false;
            for (int i = 0; i < density; i++) {
                if (phase == blackPhases[density - 1][i]) {
                    black = true;
                    break;
                }
            }
            if (!black) {
                row |= 0x80 >> screenX;
            }
        }
        pattern[screenY] = row;
        pattern[screenY + 8] = 0xff;
    }

    pd->graphics->fillRect(
        x,
        y,
        width,
        height,
        reinterpret_cast<LCDColor>(pattern)
    );
}

float PDGetCrankChange(void)
{
    return crankChange;
}

void RequestQuitToTitle(void)
{
    // Playdate games do not quit to a desktop. The caller changes its own
    // state to the title screen instead.
}

void UpdateKeys(void)
{
    PDButtons released = (PDButtons)0;
    pd->system->getButtonState(&heldButtons, &pushedButtons, &released);
    crankChange = pd->system->getCrankChange();
}

static PDButtons buttonForKey(int key)
{
    switch (key) {
        case KEY_INPUT_LEFT:
            return kButtonLeft;
        case KEY_INPUT_RIGHT:
            return kButtonRight;
        case KEY_INPUT_DOWN:
            return kButtonDown;
        case KEY_INPUT_UP:
            return kButtonUp;
        case KEY_INPUT_Z:
        case KEY_INPUT_RETURN:
            return kButtonA;
        case KEY_INPUT_SPACE:
            return kButtonB;
        default:
            return (PDButtons)0;
    }
}

byte CheckHitKey(int key)
{
    const PDButtons button = buttonForKey(key);
    return button != 0 && ((heldButtons | pushedButtons) & button) != 0;
}

byte ButtonJustPressed(int key)
{
    const PDButtons button = buttonForKey(key);
    return button != 0 && (pushedButtons & button) != 0;
}

byte ProcessMessage(void)
{
    return 0;
}

byte WaitKey(void)
{
    return ButtonJustPressed(KEY_INPUT_Z);
}

int SDL_JoystickGetButton(SDL_Joystick* ignored, int button)
{
    (void)ignored;
    (void)button;
    return CheckHitKey(KEY_INPUT_Z);
}

Uint32 GetColor(byte red, byte green, byte blue)
{
    return ((Uint32)red << 16) | ((Uint32)green << 8) | blue;
}

void SetFontSize(byte size)
{
    fontSize = size;
    pd->graphics->setFont(size >= 18 ? largeFont : smallFont);
}

void ChangeFontType(byte type)
{
    fontType = type;
}

int PDGetTextWidth(const char* text)
{
    if (text == nullptr)
        return 0;

    LCDFont* currentFont = fontSize >= 18 ? largeFont : smallFont;
    return pd->graphics->getTextWidth(
        currentFont,
        text,
        strlen(text),
        kUTF8Encoding,
        0
    );
}

void DrawString(int x, int y, const char* text, Uint32 color)
{
    if (!drawingEnabled || text == nullptr) {
        return;
    }

    const LCDColor lcdColor = packedRgbToColor(color);
    const LCDBitmapDrawMode textMode =
        lcdColor == kColorWhite ? kDrawModeFillWhite : kDrawModeFillBlack;

    pd->graphics->setDrawMode(textMode);

    if (fontType == DX_FONTTYPE_EDGE) {
        pd->graphics->setDrawMode(
            lcdColor == kColorWhite ? kDrawModeFillBlack : kDrawModeFillWhite
        );
        pd->graphics->drawText(text, strlen(text), kUTF8Encoding, x + 1, y + 1);
        pd->graphics->setDrawMode(textMode);
    }

    pd->graphics->drawText(text, strlen(text), kUTF8Encoding, x, y);
    pd->graphics->setDrawMode(kDrawModeCopy);
}

void DrawFormatString(int x, int y, Uint32 color, const char* format, ...)
{
    if (!drawingEnabled) {
        return;
    }

    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    DrawString(x, y, buffer, color);
}

void DrawGraphZ(int x, int y, SDL_Surface* surface)
{
    if (drawingEnabled && surface != nullptr && surface->bitmap != nullptr) {
        pd->graphics->drawBitmap(surface->bitmap, x, y, kBitmapUnflipped);
    }
}

void DrawTurnGraphZ(int x, int y, SDL_Surface* surface)
{
    if (drawingEnabled && surface != nullptr && surface->bitmap != nullptr) {
        pd->graphics->drawBitmap(surface->bitmap, x, y, kBitmapFlippedX);
    }
}

void DrawVertTurnGraph(int centerX, int centerY, SDL_Surface* surface)
{
    if (drawingEnabled && surface != nullptr && surface->bitmap != nullptr) {
        pd->graphics->drawBitmap(
            surface->bitmap,
            centerX - surface->w / 2,
            centerY - surface->h / 2,
            kBitmapFlippedXY
        );
    }
}

SDL_Surface* DerivationGraph(
    int sourceX,
    int sourceY,
    int width,
    int height,
    SDL_Surface* source
)
{
    if (source == nullptr || source->bitmap == nullptr) {
        return nullptr;
    }

    LCDBitmap* bitmap = pd->graphics->newBitmap(width, height, kColorClear);
    if (bitmap == nullptr) {
        pd->system->error("Unable to allocate %dx%d bitmap", width, height);
        return nullptr;
    }

    pd->graphics->pushContext(bitmap);
    pd->graphics->setDrawOffset(0, 0);
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->drawBitmap(
        source->bitmap,
        -sourceX,
        -sourceY,
        kBitmapUnflipped
    );
    pd->graphics->popContext();
    pd->graphics->setDrawOffset(drawOffsetX, drawOffsetY);

    SDL_Surface* surface = static_cast<SDL_Surface*>(
        pd->system->realloc(nullptr, sizeof(SDL_Surface))
    );
    surface->bitmap = bitmap;
    surface->w = width;
    surface->h = height;
    return surface;
}

SDL_Surface* LoadGraph(const char* filename)
{
    char path[192];
    assetPathWithoutExtension(filename, path, sizeof(path));

    const char* error = nullptr;
    LCDBitmap* bitmap = pd->graphics->loadBitmap(path, &error);
    if (bitmap == nullptr) {
        pd->system->error("Unable to load image %s: %s", path, error);
        return nullptr;
    }

    int width = 0;
    int height = 0;
    pd->graphics->getBitmapData(
        bitmap,
        &width,
        &height,
        nullptr,
        nullptr,
        nullptr
    );

    SDL_Surface* surface = static_cast<SDL_Surface*>(
        pd->system->realloc(nullptr, sizeof(SDL_Surface))
    );
    surface->bitmap = bitmap;
    surface->w = width;
    surface->h = height;
    return surface;
}

void SDL_FreeSurface(SDL_Surface* surface)
{
    if (surface != nullptr) {
        if (surface->bitmap != nullptr) {
            pd->graphics->freeBitmap(surface->bitmap);
        }
        pd->system->realloc(surface, 0);
    }
}

Mix_Chunk* LoadSoundMem(const char* filename)
{
    if (!sound) {
        return nullptr;
    }

    char path[192];
    assetPathWithoutExtension(filename, path, sizeof(path));
    AudioSample* sample = pd->sound->sample->load(path);
    if (sample == nullptr) {
        pd->system->logToConsole(
            "Unable to load sound %s: %s",
            path,
            pd->sound->getError()
        );
        return nullptr;
    }

    SamplePlayer* player = pd->sound->sampleplayer->newPlayer();
    pd->sound->sampleplayer->setSample(player, sample);

    Mix_Chunk* chunk = static_cast<Mix_Chunk*>(
        pd->system->realloc(nullptr, sizeof(Mix_Chunk))
    );
    chunk->sample = sample;
    chunk->player = player;
    return chunk;
}

Mix_Music* LoadMusicMem(const char* filename)
{
    if (!sound) {
        return nullptr;
    }

    char path[192];
    assetPathWithoutExtension(filename, path, sizeof(path));
    FilePlayer* player = pd->sound->fileplayer->newPlayer();
    if (!pd->sound->fileplayer->loadIntoPlayer(player, path)) {
        pd->system->logToConsole(
            "Unable to load music %s: %s",
            path,
            pd->sound->getError()
        );
        pd->sound->fileplayer->freePlayer(player);
        return nullptr;
    }

    Mix_Music* music = static_cast<Mix_Music*>(
        pd->system->realloc(nullptr, sizeof(Mix_Music))
    );
    music->player = player;
    return music;
}

void PlaySoundMem(Mix_Chunk* chunk, int loopMode)
{
    if (sound && chunk != nullptr && chunk->player != nullptr) {
        pd->sound->sampleplayer->stop(chunk->player);
        pd->sound->sampleplayer->play(
            chunk->player,
            loopMode == DX_PLAYTYPE_LOOP ? 0 : 1,
            1.0f
        );
    }
}

void StopSoundMemZ(Mix_Chunk* chunk)
{
    if (chunk != nullptr && chunk->player != nullptr) {
        pd->sound->sampleplayer->stop(chunk->player);
    }
}

void Mix_HaltChannel(int channel)
{
    (void)channel;
    // The original only uses this to silence one-shot effects on death.
    // Individual effects are short, so avoiding a 19-player scan here keeps
    // this compatibility layer independent of the game's global arrays.
}

void Mix_HaltMusic(void)
{
    if (currentMusic != nullptr && currentMusic->player != nullptr) {
        pd->sound->fileplayer->stop(currentMusic->player);
    }
    currentMusic = nullptr;
}

int Mix_PlayMusic(Mix_Music* music, int loops)
{
    if (!sound || music == nullptr || music->player == nullptr) {
        return 0;
    }

    currentMusic = music;
    return pd->sound->fileplayer->play(
        music->player,
        loops < 0 ? 0 : loops
    );
}

void Mix_VolumeMusic(int volume)
{
    if (currentMusic != nullptr && currentMusic->player != nullptr) {
        const float normalized = (float)volume / (float)MIX_MAX_VOLUME;
        pd->sound->fileplayer->setVolume(
            currentMusic->player,
            normalized,
            normalized
        );
    }
}

void Mix_FreeMusic(Mix_Music* music)
{
    if (music != nullptr) {
        if (music->player != nullptr) {
            pd->sound->fileplayer->freePlayer(music->player);
        }
        pd->system->realloc(music, 0);
    }
}

void Mix_FreeChunk(Mix_Chunk* chunk)
{
    if (chunk != nullptr) {
        if (chunk->player != nullptr) {
            pd->sound->sampleplayer->freePlayer(chunk->player);
        }
        if (chunk->sample != nullptr) {
            pd->sound->sample->freeSample(chunk->sample);
        }
        pd->system->realloc(chunk, 0);
    }
}

void pixelColor(SDL_Surface* ignored, int x, int y, Uint32 rgba)
{
    (void)ignored;
    if (drawingEnabled)
        pd->graphics->fillRect(x, y, 1, 1, packedRgbaToColor(rgba));
}

void lineColor(
    SDL_Surface* ignored,
    int x1,
    int y1,
    int x2,
    int y2,
    Uint32 rgba
)
{
    (void)ignored;
    if (drawingEnabled)
        pd->graphics->drawLine(x1, y1, x2, y2, 1, packedRgbaToColor(rgba));
}

void rectangleColor(
    SDL_Surface* ignored,
    int x1,
    int y1,
    int x2,
    int y2,
    Uint32 rgba
)
{
    (void)ignored;
    if (drawingEnabled) {
        pd->graphics->drawRect(
            x1,
            y1,
            x2 - x1 + 1,
            y2 - y1 + 1,
            packedRgbaToColor(rgba)
        );
    }
}

void boxColor(
    SDL_Surface* ignored,
    int x1,
    int y1,
    int x2,
    int y2,
    Uint32 rgba
)
{
    (void)ignored;
    if (drawingEnabled) {
        pd->graphics->fillRect(
            x1,
            y1,
            x2 - x1 + 1,
            y2 - y1 + 1,
            packedRgbaToColor(rgba)
        );
    }
}

void ellipseColor(
    SDL_Surface* ignored,
    int centerX,
    int centerY,
    int radiusX,
    int radiusY,
    Uint32 rgba
)
{
    (void)ignored;
    if (drawingEnabled) {
        pd->graphics->drawEllipse(
            centerX - radiusX,
            centerY - radiusY,
            radiusX * 2,
            radiusY * 2,
            1,
            0,
            360,
            packedRgbaToColor(rgba)
        );
    }
}

void filledEllipseColor(
    SDL_Surface* ignored,
    int centerX,
    int centerY,
    int radiusX,
    int radiusY,
    Uint32 rgba
)
{
    (void)ignored;
    if (drawingEnabled) {
        pd->graphics->fillEllipse(
            centerX - radiusX,
            centerY - radiusY,
            radiusX * 2,
            radiusY * 2,
            0,
            360,
            packedRgbaToColor(rgba)
        );
    }
}

void SDL_FillRect(SDL_Surface* ignored, void* rect, Uint32 rgb)
{
    (void)ignored;
    (void)rect;
    if (drawingEnabled)
        pd->graphics->clear(packedRgbToColor(rgb));
}

void SDL_Flip(SDL_Surface* ignored) { (void)ignored; }
void SDL_JoystickClose(SDL_Joystick* ignored) { (void)ignored; }
void TTF_CloseFont(TTF_Font* ignored) { (void)ignored; }
void IMG_Quit(void) {}
void TTF_Quit(void) {}
void Mix_Quit(void) {}
void SDL_Quit(void) {}
