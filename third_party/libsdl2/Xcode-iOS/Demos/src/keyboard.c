/*
 *  keyboard.c
 *  written by Holmes Futrell
 *  use however you want
 */

#include "SDL.h"
#include "common.h"

#define TEST_INPUT_RECT

#define GLYPH_SIZE_IMAGE 16     /* size of glyphs (characters) in the bitmap font file */
#define GLYPH_SIZE_SCREEN 32    /* size of glyphs (characters) as shown on the screen */

#define MAX_CHARS 1024

static SDL_Texture *texture; /* texture where we'll hold our font */

static SDL_Renderer *renderer;
static int numChars = 0;        /* number of characters we've typed so far */
static SDL_Color bg_color = { 50, 50, 100, 255 };       /* color of background */

static int glyphs[MAX_CHARS];

/* this structure maps a scancode to an index in our bitmap font.
   it also contains data about under which modifiers the mapping is valid
   (for example, we don't want shift + 1 to produce the character '1',
   but rather the character '!')
*/
typedef struct
{
    SDL_Scancode scancode;      /* scancode of the key we want to map */
    int allow_no_mod;           /* is the map valid if the key has no modifiers? */
    SDL_Keymod mod;             /* what modifiers are allowed for the mapping */
    int index;                  /* what index in the font does the scancode map to */
} fontMapping;

#define TABLE_SIZE 51           /* size of our table which maps keys and modifiers to font indices */

/* Below is the table that defines the mapping between scancodes and modifiers to indices in the
   bitmap font.  As an example, then line '{ SDL_SCANCODE_A, 1, KMOD_SHIFT, 33 }' means, map
   the key A (which has scancode SDL_SCANCODE_A) to index 33 in the font (which is a picture of an A),
   The '1' means that the mapping is valid even if there are no modifiers, and KMOD_SHIFT means the
   mapping is also valid if the user is holding shift.
*/
fontMapping map[TABLE_SIZE] = {

    {SDL_SCANCODE_A, 1, KMOD_SHIFT, 33},        /* A */
    {SDL_SCANCODE_B, 1, KMOD_SHIFT, 34},        /* B */
    {SDL_SCANCODE_C, 1, KMOD_SHIFT, 35},        /* C */
    {SDL_SCANCODE_D, 1, KMOD_SHIFT, 36},        /* D */
    {SDL_SCANCODE_E, 1, KMOD_SHIFT, 37},        /* E */
    {SDL_SCANCODE_F, 1, KMOD_SHIFT, 38},        /* F */
    {SDL_SCANCODE_G, 1, KMOD_SHIFT, 39},        /* G */
    {SDL_SCANCODE_H, 1, KMOD_SHIFT, 40},        /* H */
    {SDL_SCANCODE_I, 1, KMOD_SHIFT, 41},        /* I */
    {SDL_SCANCODE_J, 1, KMOD_SHIFT, 42},        /* J */
    {SDL_SCANCODE_K, 1, KMOD_SHIFT, 43},        /* K */
    {SDL_SCANCODE_L, 1, KMOD_SHIFT, 44},        /* L */
    {SDL_SCANCODE_M, 1, KMOD_SHIFT, 45},        /* M */
    {SDL_SCANCODE_N, 1, KMOD_SHIFT, 46},        /* N */
    {SDL_SCANCODE_O, 1, KMOD_SHIFT, 47},        /* O */
    {SDL_SCANCODE_P, 1, KMOD_SHIFT, 48},        /* P */
    {SDL_SCANCODE_Q, 1, KMOD_SHIFT, 49},        /* Q */
    {SDL_SCANCODE_R, 1, KMOD_SHIFT, 50},        /* R */
    {SDL_SCANCODE_S, 1, KMOD_SHIFT, 51},        /* S */
    {SDL_SCANCODE_T, 1, KMOD_SHIFT, 52},        /* T */
    {SDL_SCANCODE_U, 1, KMOD_SHIFT, 53},        /* U */
    {SDL_SCANCODE_V, 1, KMOD_SHIFT, 54},        /* V */
    {SDL_SCANCODE_W, 1, KMOD_SHIFT, 55},        /* W */
    {SDL_SCANCODE_X, 1, KMOD_SHIFT, 56},        /* X */
    {SDL_SCANCODE_Y, 1, KMOD_SHIFT, 57},        /* Y */
    {SDL_SCANCODE_Z, 1, KMOD_SHIFT, 58},        /* Z */
    {SDL_SCANCODE_0, 1, 0, 16}, /* 0 */
    {SDL_SCANCODE_1, 1, 0, 17}, /* 1 */
    {SDL_SCANCODE_2, 1, 0, 18}, /* 2 */
    {SDL_SCANCODE_3, 1, 0, 19}, /* 3 */
    {SDL_SCANCODE_4, 1, 0, 20}, /* 4 */
    {SDL_SCANCODE_5, 1, 0, 21}, /* 5 */
    {SDL_SCANCODE_6, 1, 0, 22}, /* 6 */
    {SDL_SCANCODE_7, 1, 0, 23}, /* 7 */
    {SDL_SCANCODE_8, 1, 0, 24}, /* 8 */
    {SDL_SCANCODE_9, 1, 0, 25}, /* 9 */
    {SDL_SCANCODE_SPACE, 1, 0, 0},      /* ' ' */
    {SDL_SCANCODE_1, 0, KMOD_SHIFT, 1}, /* ! */
    {SDL_SCANCODE_SLASH, 0, KMOD_SHIFT, 31},    /* ? */
    {SDL_SCANCODE_SLASH, 1, 0, 15},     /* / */
    {SDL_SCANCODE_COMMA, 1, 0, 12},     /* , */
    {SDL_SCANCODE_SEMICOLON, 1, 0, 27}, /* ; */
    {SDL_SCANCODE_SEMICOLON, 0, KMOD_SHIFT, 26},        /* : */
    {SDL_SCANCODE_PERIOD, 1, 0, 14},    /* . */
    {SDL_SCANCODE_MINUS, 1, 0, 13},     /* - */
    {SDL_SCANCODE_EQUALS, 0, KMOD_SHIFT, 11},   /* = */
    {SDL_SCANCODE_APOSTROPHE, 1, 0, 7}, /* ' */
    {SDL_SCANCODE_APOSTROPHE, 0, KMOD_SHIFT, 2},        /* " */
    {SDL_SCANCODE_5, 0, KMOD_SHIFT, 5}, /* % */

};

/*
    This function maps an SDL_KeySym to an index in the bitmap font.
    It does so by scanning through the font mapping table one entry
    at a time.

    If a match is found (scancode and allowed modifiers), the proper
    index is returned.

    If there is no entry for the key, -1 is returned
*/
int
keyToGlyphIndex(SDL_Keysym key)
{
    int i, index = -1;
    for (i = 0; i < TABLE_SIZE; i++) {
        fontMapping compare = map[i];
        if (key.scancode == compare.scancode) {
            /* if this entry is valid with no key mod and we have no keymod, or if
               the key's modifiers are allowed modifiers for that mapping */
            if ((compare.allow_no_mod && key.mod == 0)
                || (key.mod & compare.mod)) {
                index = compare.index;
                break;
            }
        }
    }
    return index;
}

/*
    This function returns and x,y position for a given character number.
    It is used for positioning each character of text
*/
void
getPositionForCharNumber(int n, int *x, int *y)
{
    int renderW, renderH;
    SDL_RenderGetLogicalSize(renderer, &renderW, &renderH);

    int x_padding = 16;         /* padding space on left and right side of screen */
    int y_padding = 32;         /* padding space at top of screen */
    /* figure out the number of characters that can fit horizontally across the screen */
    int max_x_chars = (renderW - 2 * x_padding) / GLYPH_SIZE_SCREEN;
    int line_separation = 5;    /* pixels between each line */
    *x = (n % max_x_chars) * GLYPH_SIZE_SCREEN + x_padding;
#ifdef TEST_INPUT_RECT
    *y = renderH - GLYPH_SIZE_SCREEN;
#else
    *y = (n / max_x_chars) * (GLYPH_SIZE_SCREEN + line_separation) + y_padding;
#endif
}

void
drawGlyph(int glyph, int positionIndex)
{
    int x, y;
    getPositionForCharNumber(positionIndex, &x, &y);
    SDL_Rect srcRect = { GLYPH_SIZE_IMAGE * glyph, 0, GLYPH_SIZE_IMAGE, GLYPH_SIZE_IMAGE };
    SDL_Rect dstRect = { x, y, GLYPH_SIZE_SCREEN, GLYPH_SIZE_SCREEN };
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
}

/* this function loads our font into an SDL_Texture and returns the SDL_Texture  */
SDL_Texture*
loadFont(void)
{
    SDL_Surface *surface = SDL_LoadBMP("kromasky_16x16.bmp");

    if (!surface) {
        printf("Error loading bitmap: %s\n", SDL_GetError());
        return 0;
    } else {
        /* set the transparent color for the bitmap font (hot pink) */
        SDL_SetColorKey(surface, 1, SDL_MapRGB(surface->format, 238, 0, 252));
        /* now we convert the surface to our desired pixel format */
        int format = SDL_PIXELFORMAT_ABGR8888;  /* desired texture format */
        Uint32 Rmask, Gmask, Bmask, Amask;      /* masks for desired format */
        int bpp;                /* bits per pixel for desired format */
        SDL_PixelFormatEnumToMasks(format, &bpp, &Rmask, &Gmask, &Bmask,
                                   &Amask);
        SDL_Surface *converted =
            SDL_CreateRGBSurface(0, surface->w, surface->h, bpp, Rmask, Gmask,
                                 Bmask, Amask);
        SDL_BlitSurface(surface, NULL, converted, NULL);
        /* create our texture */
        texture = SDL_CreateTextureFromSurface(renderer, converted);
        if (texture == 0) {
            printf("texture creation failed: %s\n", SDL_GetError());
        } else {
            /* set blend mode for our texture */
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        }
        SDL_FreeSurface(surface);
        SDL_FreeSurface(converted);
        return texture;
    }
}

void
draw()
{
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderClear(renderer);

    for (int i = 0; i < numChars; i++) {
        drawGlyph(glyphs[i], i);
    }

    drawGlyph(29, numChars); /* cursor is at index 29 in the bitmap font */

    SDL_RenderPresent(renderer);
}

int
main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event event;            /* last event received */
    SDL_Scancode scancode;      /* scancode of last key we pushed */
    int width;
    int height;
    int done;
    SDL_Rect textrect;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error initializing SDL: %s", SDL_GetError());
    }
    /* create window */
    window = SDL_CreateWindow("iOS keyboard test", 0, 0, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    /* create renderer */
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    SDL_GetWindowSize(window, &width, &height);
    SDL_RenderSetLogicalSize(renderer, width, height);

    /* load up our font */
    loadFont();
    
    /* Show onscreen keyboard */
#ifdef TEST_INPUT_RECT
    textrect.x = 0;
    textrect.y = height - GLYPH_SIZE_IMAGE;
    textrect.w = width;
    textrect.h = GLYPH_SIZE_IMAGE;
    SDL_SetTextInputRect(&textrect);
#endif
    SDL_StartTextInput();

    done = 0;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                done = 1;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					width = event.window.data1;
					height = event.window.data2;
                    SDL_RenderSetLogicalSize(renderer, width, height);
#ifdef TEST_INPUT_RECT
                    textrect.x = 0;
                    textrect.y = height - GLYPH_SIZE_IMAGE;
                    textrect.w = width;
                    textrect.h = GLYPH_SIZE_IMAGE;
                    SDL_SetTextInputRect(&textrect);
#endif
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
                    if (numChars > 0) {
                        numChars--;
                    }
                } else if (numChars + 1 < MAX_CHARS) {
                    int index = keyToGlyphIndex(event.key.keysym);
                    if (index >= 0) {
                        glyphs[numChars++] = index;
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                /* mouse up toggles onscreen keyboard visibility */
                if (SDL_IsTextInputActive()) {
                    SDL_StopTextInput();
                } else {
                    SDL_StartTextInput();
                }
                break;
            }
        }

        draw();
        SDL_Delay(15);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
