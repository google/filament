/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
/* A simple program to test the Input Method support in the SDL library (2.0+)
   If you build without SDL_ttf, you can use the GNU Unifont hex file instead.
   Download at http://unifoundry.com/unifont.html */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "SDL.h"
#ifdef HAVE_SDL_TTF
#include "SDL_ttf.h"
#endif

#include "SDL_test_common.h"

#define DEFAULT_PTSIZE 30
#ifdef HAVE_SDL_TTF
#ifdef __MACOSX__
#define DEFAULT_FONT "/System/Library/Fonts/华文细黑.ttf"
#elif __WIN32__
/* Some japanese font present on at least Windows 8.1. */
#define DEFAULT_FONT "C:\\Windows\\Fonts\\yugothic.ttf"
#else
#define DEFAULT_FONT "NoDefaultFont.ttf"
#endif
#else
#define DEFAULT_FONT "unifont-13.0.06.hex"
#endif
#define MAX_TEXT_LENGTH 256

static SDLTest_CommonState *state;
static SDL_Rect textRect, markedRect;
static SDL_Color lineColor = {0,0,0,255};
static SDL_Color backColor = {255,255,255,255};
static SDL_Color textColor = {0,0,0,255};
static char text[MAX_TEXT_LENGTH], markedText[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
static int cursor = 0;
#ifdef HAVE_SDL_TTF
static TTF_Font *font;
#else
#define UNIFONT_MAX_CODEPOINT 0x1ffff
#define UNIFONT_NUM_GLYPHS 0x20000
/* Using 512x512 textures that are supported everywhere. */
#define UNIFONT_TEXTURE_WIDTH 512
#define UNIFONT_GLYPHS_IN_ROW (UNIFONT_TEXTURE_WIDTH / 16)
#define UNIFONT_GLYPHS_IN_TEXTURE (UNIFONT_GLYPHS_IN_ROW * UNIFONT_GLYPHS_IN_ROW)
#define UNIFONT_NUM_TEXTURES ((UNIFONT_NUM_GLYPHS + UNIFONT_GLYPHS_IN_TEXTURE - 1) / UNIFONT_GLYPHS_IN_TEXTURE)
#define UNIFONT_TEXTURE_SIZE (UNIFONT_TEXTURE_WIDTH * UNIFONT_TEXTURE_WIDTH * 4)
#define UNIFONT_TEXTURE_PITCH (UNIFONT_TEXTURE_WIDTH * 4)
#define UNIFONT_DRAW_SCALE 2
struct UnifontGlyph {
    Uint8 width;
    Uint8 data[32];
} *unifontGlyph;
static SDL_Texture **unifontTexture;
static Uint8 unifontTextureLoaded[UNIFONT_NUM_TEXTURES] = {0};

/* Unifont loading code start */

static Uint8 dehex(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 255;
}

static Uint8 dehex2(char c1, char c2)
{
    return (dehex(c1) << 4) | dehex(c2);
}

static Uint8 validate_hex(const char *cp, size_t len, Uint32 *np)
{
    Uint32 n = 0;
    for (; len > 0; cp++, len--)
    {
        Uint8 c = dehex(*cp);
        if (c == 255)
            return 0;
        n = (n << 4) | c;
    }
    if (np != NULL)
        *np = n;
    return 1;
}

static int unifont_init(const char *fontname)
{
    Uint8 hexBuffer[65];
    Uint32 numGlyphs = 0;
    int lineNumber = 1;
    size_t bytesRead;
    SDL_RWops *hexFile;
    const size_t unifontGlyphSize = UNIFONT_NUM_GLYPHS * sizeof(struct UnifontGlyph);
    const size_t unifontTextureSize = UNIFONT_NUM_TEXTURES * state->num_windows * sizeof(void *);

    /* Allocate memory for the glyph data so the file can be closed after initialization. */
    unifontGlyph = (struct UnifontGlyph *)SDL_malloc(unifontGlyphSize);
    if (unifontGlyph == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Failed to allocate %d KiB for glyph data.\n", (int)(unifontGlyphSize + 1023) / 1024);
        return -1;
    }
    SDL_memset(unifontGlyph, 0, unifontGlyphSize);

    /* Allocate memory for texture pointers for all renderers. */
    unifontTexture = (SDL_Texture **)SDL_malloc(unifontTextureSize);
    if (unifontTexture == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Failed to allocate %d KiB for texture pointer data.\n", (int)(unifontTextureSize + 1023) / 1024);
        return -1;
    }
    SDL_memset(unifontTexture, 0, unifontTextureSize);

    hexFile = SDL_RWFromFile(fontname, "rb");
    if (hexFile == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Failed to open font file: %s\n", fontname);
        return -1;
    }

    /* Read all the glyph data into memory to make it accessible later when textures are created. */
    do {
        int i, codepointHexSize;
        size_t bytesOverread;
        Uint8 glyphWidth;
        Uint32 codepoint;

        bytesRead = SDL_RWread(hexFile, hexBuffer, 1, 9);
        if (numGlyphs > 0 && bytesRead == 0)
            break; /* EOF */
        if ((numGlyphs == 0 && bytesRead == 0) || (numGlyphs > 0 && bytesRead < 9))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Unexpected end of hex file.\n");
            return -1;
        }

        /* Looking for the colon that separates the codepoint and glyph data at position 2, 4, 6 and 8. */
        if (hexBuffer[2] == ':')
            codepointHexSize = 2;
        else if (hexBuffer[4] == ':')
            codepointHexSize = 4;
        else if (hexBuffer[6] == ':')
            codepointHexSize = 6;
        else if (hexBuffer[8] == ':')
            codepointHexSize = 8;
        else
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Could not find codepoint and glyph data separator symbol in hex file on line %d.\n", lineNumber);
            return -1;
        }

        if (!validate_hex((const char *)hexBuffer, codepointHexSize, &codepoint))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Malformed hexadecimal number in hex file on line %d.\n", lineNumber);
            return -1;
        }
        if (codepoint > UNIFONT_MAX_CODEPOINT)
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unifont: Codepoint on line %d exceeded limit of 0x%x.\n", lineNumber, UNIFONT_MAX_CODEPOINT);

        /* If there was glyph data read in the last file read, move it to the front of the buffer. */
        bytesOverread = 8 - codepointHexSize;
        if (codepointHexSize < 8)
            SDL_memmove(hexBuffer, hexBuffer + codepointHexSize + 1, bytesOverread);
        bytesRead = SDL_RWread(hexFile, hexBuffer + bytesOverread, 1, 33 - bytesOverread);
        if (bytesRead < (33 - bytesOverread))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Unexpected end of hex file.\n");
            return -1;
        }
        if (hexBuffer[32] == '\n')
            glyphWidth = 8;
        else
        {
            glyphWidth = 16;
            bytesRead = SDL_RWread(hexFile, hexBuffer + 33, 1, 32);
            if (bytesRead < 32)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Unexpected end of hex file.\n");
                return -1;
            }
        }

        if (!validate_hex((const char *)hexBuffer, glyphWidth * 4, NULL))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Malformed hexadecimal glyph data in hex file on line %d.\n", lineNumber);
            return -1;
        }

        if (codepoint <= UNIFONT_MAX_CODEPOINT)
        {
            if (unifontGlyph[codepoint].width > 0)
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "unifont: Ignoring duplicate codepoint 0x%08x in hex file on line %d.\n", codepoint, lineNumber);
            else
            {
                unifontGlyph[codepoint].width = glyphWidth;
                /* Pack the hex data into a more compact form. */
                for (i = 0; i < glyphWidth * 2; i++)
                    unifontGlyph[codepoint].data[i] = dehex2(hexBuffer[i * 2], hexBuffer[i * 2 + 1]);
                numGlyphs++;
            }
        }

        lineNumber++;
    } while (bytesRead > 0);

    SDL_RWclose(hexFile);
    SDL_Log("unifont: Loaded %u glyphs.\n", numGlyphs);
    return 0;
}

static void unifont_make_rgba(Uint8 *src, Uint8 *dst, Uint8 width)
{
    int i, j;
    Uint8 *row = dst;

    for (i = 0; i < width * 2; i++)
    {
        Uint8 data = src[i];
        for (j = 0; j < 8; j++)
        {
            if (data & 0x80)
            {
                row[0] = textColor.r;
                row[1] = textColor.g;
                row[2] = textColor.b;
                row[3] = textColor.a;
            }
            else
            {
                row[0] = 0;
                row[1] = 0;
                row[2] = 0;
                row[3] = 0;
            }
            data <<= 1;
            row += 4;
        }

        if (width == 8 || (width == 16 && i % 2 == 1))
        {
            dst += UNIFONT_TEXTURE_PITCH;
            row = dst;
        }
    }
}

static int unifont_load_texture(Uint32 textureID)
{
    int i;
    Uint8 * textureRGBA;

    if (textureID >= UNIFONT_NUM_TEXTURES)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Tried to load out of range texture %u.\n", textureID);
        return -1;
    }

    textureRGBA = (Uint8 *)SDL_malloc(UNIFONT_TEXTURE_SIZE);
    if (textureRGBA == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Failed to allocate %d MiB for a texture.\n", UNIFONT_TEXTURE_SIZE / 1024 / 1024);
        return -1;
    }
    SDL_memset(textureRGBA, 0, UNIFONT_TEXTURE_SIZE);

    /* Copy the glyphs into memory in RGBA format. */
    for (i = 0; i < UNIFONT_GLYPHS_IN_TEXTURE; i++)
    {
        Uint32 codepoint = UNIFONT_GLYPHS_IN_TEXTURE * textureID + i;
        if (unifontGlyph[codepoint].width > 0)
        {
            const Uint32 cInTex = codepoint % UNIFONT_GLYPHS_IN_TEXTURE;
            const size_t offset = (cInTex / UNIFONT_GLYPHS_IN_ROW) * UNIFONT_TEXTURE_PITCH * 16 + (cInTex % UNIFONT_GLYPHS_IN_ROW) * 16 * 4;
            unifont_make_rgba(unifontGlyph[codepoint].data, textureRGBA + offset, unifontGlyph[codepoint].width);
        }
    }

    /* Create textures and upload the RGBA data from above. */
    for (i = 0; i < state->num_windows; ++i)
    {
        SDL_Renderer *renderer = state->renderers[i];
        SDL_Texture *tex = unifontTexture[UNIFONT_NUM_TEXTURES * i + textureID];
        if (state->windows[i] == NULL || renderer == NULL || tex != NULL)
            continue;
        tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, UNIFONT_TEXTURE_WIDTH, UNIFONT_TEXTURE_WIDTH);
        if (tex == NULL)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "unifont: Failed to create texture %u for renderer %d.\n", textureID, i);
            return -1;
        }
        unifontTexture[UNIFONT_NUM_TEXTURES * i + textureID] = tex;
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        if (SDL_UpdateTexture(tex, NULL, textureRGBA, UNIFONT_TEXTURE_PITCH) != 0)
        {
            SDL_Log("unifont error: Failed to update texture %u data for renderer %d.\n", textureID, i);
        }
    }

    SDL_free(textureRGBA);
    unifontTextureLoaded[textureID] = 1;
    return 0;
}

static Sint32 unifont_draw_glyph(Uint32 codepoint, int rendererID, SDL_Rect *dstrect)
{
    SDL_Texture *texture;
    const Uint32 textureID = codepoint / UNIFONT_GLYPHS_IN_TEXTURE;
    SDL_Rect srcrect;
    srcrect.w = srcrect.h = 16;
    if (codepoint > UNIFONT_MAX_CODEPOINT) {
        return 0;
    }
    if (!unifontTextureLoaded[textureID]) {
        if (unifont_load_texture(textureID) < 0) {
            return 0;
        }
    }
    texture = unifontTexture[UNIFONT_NUM_TEXTURES * rendererID + textureID];
    if (texture != NULL)
    {
        const Uint32 cInTex = codepoint % UNIFONT_GLYPHS_IN_TEXTURE;
        srcrect.x = cInTex % UNIFONT_GLYPHS_IN_ROW * 16;
        srcrect.y = cInTex / UNIFONT_GLYPHS_IN_ROW * 16;
        SDL_RenderCopy(state->renderers[rendererID], texture, &srcrect, dstrect);
    }
    return unifontGlyph[codepoint].width;
}

static void unifont_cleanup()
{
    int i, j;
    for (i = 0; i < state->num_windows; ++i)
    {
        SDL_Renderer *renderer = state->renderers[i];
        if (state->windows[i] == NULL || renderer == NULL)
            continue;
        for (j = 0; j < UNIFONT_NUM_TEXTURES; j++)
        {
            SDL_Texture *tex = unifontTexture[UNIFONT_NUM_TEXTURES * i + j];
            if (tex != NULL)
                SDL_DestroyTexture(tex);
        }
    }

    for (j = 0; j < UNIFONT_NUM_TEXTURES; j++)
          unifontTextureLoaded[j] = 0;

    SDL_free(unifontTexture);
    SDL_free(unifontGlyph);
}

/* Unifont code end */
#endif

size_t utf8_length(unsigned char c)
{
    c = (unsigned char)(0xff & c);
    if (c < 0x80)
        return 1;
    else if ((c >> 5) ==0x6)
        return 2;
    else if ((c >> 4) == 0xe)
        return 3;
    else if ((c >> 3) == 0x1e)
        return 4;
    else
        return 0;
}

char *utf8_next(char *p)
{
    size_t len = utf8_length(*p);
    size_t i = 0;
    if (!len)
        return 0;

    for (; i < len; ++i)
    {
        ++p;
        if (!*p)
            return 0;
    }
    return p;
}

char *utf8_advance(char *p, size_t distance)
{
    size_t i = 0;
    for (; i < distance && p; ++i)
    {
        p = utf8_next(p);
    }
    return p;
}

Uint32 utf8_decode(char *p, size_t len)
{
    Uint32 codepoint = 0;
    size_t i = 0;
    if (!len)
        return 0;

    for (; i < len; ++i)
    {
        if (i == 0)
            codepoint = (0xff >> len) & *p;
        else
        {
            codepoint <<= 6;
            codepoint |= 0x3f & *p;
        }
        if (!*p)
            return 0;
        p++;
    }

    return codepoint;
}

void usage()
{
    SDL_Log("usage: testime [--font fontfile]\n");
}

void InitInput()
{
    /* Prepare a rect for text input */
    textRect.x = textRect.y = 100;
    textRect.w = DEFAULT_WINDOW_WIDTH - 2 * textRect.x;
    textRect.h = 50;

    text[0] = 0;
    markedRect = textRect;
    markedText[0] = 0;

    SDL_StartTextInput();
}

void CleanupVideo()
{
    SDL_StopTextInput();
#ifdef HAVE_SDL_TTF
    TTF_CloseFont(font);
    TTF_Quit();
#else
    unifont_cleanup();
#endif
}

void _Redraw(int rendererID)
{
    SDL_Renderer * renderer = state->renderers[rendererID];
    SDL_Rect drawnTextRect, cursorRect, underlineRect;
    drawnTextRect = textRect;
    drawnTextRect.w = 0;

    SDL_SetRenderDrawColor(renderer, backColor.r, backColor.g, backColor.b, backColor.a);
    SDL_RenderFillRect(renderer,&textRect);

    if (*text)
    {
#ifdef HAVE_SDL_TTF
        SDL_Surface *textSur = TTF_RenderUTF8_Blended(font, text, textColor);
        SDL_Texture *texture;

        /* Vertically center text */
        drawnTextRect.y = textRect.y + (textRect.h - textSur->h) / 2;
        drawnTextRect.w = textSur->w;
        drawnTextRect.h = textSur->h;

        texture = SDL_CreateTextureFromSurface(renderer,textSur);
        SDL_FreeSurface(textSur);

        SDL_RenderCopy(renderer,texture,NULL,&drawnTextRect);
        SDL_DestroyTexture(texture);
#else
        char *utext = text;
        Uint32 codepoint;
        size_t len;
        SDL_Rect dstrect;

        dstrect.x = textRect.x;
        dstrect.y = textRect.y + (textRect.h - 16 * UNIFONT_DRAW_SCALE) / 2;
        dstrect.w = 16 * UNIFONT_DRAW_SCALE;
        dstrect.h = 16 * UNIFONT_DRAW_SCALE;
        drawnTextRect.y = dstrect.y;
        drawnTextRect.h = dstrect.h;

        while ((codepoint = utf8_decode(utext, len = utf8_length(*utext))))
        {
            Sint32 advance = unifont_draw_glyph(codepoint, rendererID, &dstrect) * UNIFONT_DRAW_SCALE;
            dstrect.x += advance;
            drawnTextRect.w += advance;
            utext += len;
        }
#endif
    }

    markedRect.x = textRect.x + drawnTextRect.w;
    markedRect.w = textRect.w - drawnTextRect.w;
    if (markedRect.w < 0)
    {
        /* Stop text input because we cannot hold any more characters */
        SDL_StopTextInput();
        return;
    }
    else
    {
        SDL_StartTextInput();
    }

    cursorRect = drawnTextRect;
    cursorRect.x += cursorRect.w;
    cursorRect.w = 2;
    cursorRect.h = drawnTextRect.h;

    drawnTextRect.x += drawnTextRect.w;
    drawnTextRect.w = 0;

    SDL_SetRenderDrawColor(renderer, backColor.r, backColor.g, backColor.b, backColor.a);
    SDL_RenderFillRect(renderer,&markedRect);

    if (markedText[0])
    {
#ifdef HAVE_SDL_TTF
        SDL_Surface *textSur;
        SDL_Texture *texture;
        if (cursor)
        {
            char *p = utf8_advance(markedText, cursor);
            char c = 0;
            if (!p)
                p = &markedText[SDL_strlen(markedText)];

            c = *p;
            *p = 0;
            TTF_SizeUTF8(font, markedText, &drawnTextRect.w, NULL);
            cursorRect.x += drawnTextRect.w;
            *p = c;
        }
        textSur = TTF_RenderUTF8_Blended(font, markedText, textColor);
        /* Vertically center text */
        drawnTextRect.y = textRect.y + (textRect.h - textSur->h) / 2;
        drawnTextRect.w = textSur->w;
        drawnTextRect.h = textSur->h;

        texture = SDL_CreateTextureFromSurface(renderer,textSur);
        SDL_FreeSurface(textSur);

        SDL_RenderCopy(renderer,texture,NULL,&drawnTextRect);
        SDL_DestroyTexture(texture);
#else
        int i = 0;
        char *utext = markedText;
        Uint32 codepoint;
        size_t len;
        SDL_Rect dstrect;

        dstrect.x = drawnTextRect.x;
        dstrect.y = textRect.y + (textRect.h - 16 * UNIFONT_DRAW_SCALE) / 2;
        dstrect.w = 16 * UNIFONT_DRAW_SCALE;
        dstrect.h = 16 * UNIFONT_DRAW_SCALE;
        drawnTextRect.y = dstrect.y;
        drawnTextRect.h = dstrect.h;

        while ((codepoint = utf8_decode(utext, len = utf8_length(*utext))))
        {
            Sint32 advance = unifont_draw_glyph(codepoint, rendererID, &dstrect) * UNIFONT_DRAW_SCALE;
            dstrect.x += advance;
            drawnTextRect.w += advance;
            if (i < cursor)
                cursorRect.x += advance;
            i++;
            utext += len;
        }
#endif

        if (cursor > 0)
        {
            cursorRect.y = drawnTextRect.y;
            cursorRect.h = drawnTextRect.h;
        }

        underlineRect = markedRect;
        underlineRect.y = drawnTextRect.y + drawnTextRect.h - 2;
        underlineRect.h = 2;
        underlineRect.w = drawnTextRect.w;

        SDL_SetRenderDrawColor(renderer, lineColor.r, lineColor.g, lineColor.b, lineColor.a);
        SDL_RenderFillRect(renderer, &underlineRect);
    }

    SDL_SetRenderDrawColor(renderer, lineColor.r, lineColor.g, lineColor.b, lineColor.a);
    SDL_RenderFillRect(renderer,&cursorRect);

    SDL_SetTextInputRect(&markedRect);
}

void Redraw()
{
    int i;
    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        if (state->windows[i] == NULL)
            continue;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        /* Sending in the window id to let the font renderers know which one we're working with. */
        _Redraw(i);

        SDL_RenderPresent(renderer);
    }
}

int main(int argc, char *argv[])
{
    int i, done;
    SDL_Event event;
    const char *fontname = DEFAULT_FONT;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if (!state) {
        return 1;
    }
    for (i = 1; i < argc;i++) {
        SDLTest_CommonArg(state, i);
    }
    for (argc--, argv++; argc > 0; argc--, argv++)
    {
        if (strcmp(argv[0], "--help") == 0) {
            usage();
            return 0;
        }

        else if (strcmp(argv[0], "--font") == 0)
        {
            argc--;
            argv++;

            if (argc > 0)
                fontname = argv[0];
            else {
                usage();
                return 0;
            }
        }
    }

    if (!SDLTest_CommonInit(state)) {
        return 2;
    }


#ifdef HAVE_SDL_TTF
    /* Initialize fonts */
    TTF_Init();

    font = TTF_OpenFont(fontname, DEFAULT_PTSIZE);
    if (! font)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to find font: %s\n", TTF_GetError());
        return -1;
    }
#else
    if (unifont_init(fontname) < 0) {
        return -1;
    }
#endif

    SDL_Log("Using font: %s\n", fontname);

    InitInput();
    /* Create the windows and initialize the renderers */
    for (i = 0; i < state->num_windows; ++i) {
        SDL_Renderer *renderer = state->renderers[i];
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
        SDL_RenderClear(renderer);
    }
    Redraw();
    /* Main render loop */
    done = 0;
    while (!done) {
        /* Check for events */
        while (SDL_PollEvent(&event)) {
            SDLTest_CommonEvent(state, &event, &done);
            switch(event.type) {
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_RETURN:
                             text[0]=0x00;
                             Redraw();
                             break;
                        case SDLK_BACKSPACE:
                            /* Only delete text if not in editing mode. */
                             if (!markedText[0])
                             {
                                 size_t textlen = SDL_strlen(text);

                                 do {
                                     if (textlen==0)
                                     {
                                         break;
                                     }
                                     if ((text[textlen-1] & 0x80) == 0x00)
                                     {
                                         /* One byte */
                                         text[textlen-1]=0x00;
                                         break;
                                     }
                                     if ((text[textlen-1] & 0xC0) == 0x80)
                                     {
                                         /* Byte from the multibyte sequence */
                                         text[textlen-1]=0x00;
                                         textlen--;
                                     }
                                     if ((text[textlen-1] & 0xC0) == 0xC0)
                                     {
                                         /* First byte of multibyte sequence */
                                         text[textlen-1]=0x00;
                                         break;
                                     }
                                 } while(1);

                                 Redraw();
                             }
                             break;
                    }

                    if (done)
                    {
                        break;
                    }

                    SDL_Log("Keyboard: scancode 0x%08X = %s, keycode 0x%08X = %s\n",
                            event.key.keysym.scancode,
                            SDL_GetScancodeName(event.key.keysym.scancode),
                            event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym));
                    break;

                case SDL_TEXTINPUT:
                    if (event.text.text[0] == '\0' || event.text.text[0] == '\n' ||
                        markedRect.w < 0)
                        break;

                    SDL_Log("Keyboard: text input \"%s\"\n", event.text.text);

                    if (SDL_strlen(text) + SDL_strlen(event.text.text) < sizeof(text))
                        SDL_strlcat(text, event.text.text, sizeof(text));

                    SDL_Log("text inputed: %s\n", text);

                    /* After text inputed, we can clear up markedText because it */
                    /* is committed */
                    markedText[0] = 0;
                    Redraw();
                    break;

                case SDL_TEXTEDITING:
                    SDL_Log("text editing \"%s\", selected range (%d, %d)\n",
                            event.edit.text, event.edit.start, event.edit.length);

                    SDL_strlcpy(markedText, event.edit.text, SDL_TEXTEDITINGEVENT_TEXT_SIZE);
                    cursor = event.edit.start;
                    Redraw();
                    break;
                }
                break;

            }
        }
    }
    CleanupVideo();
    SDLTest_CommonQuit(state);
    return 0;
}


/* vi: set ts=4 sw=4 expandtab: */
