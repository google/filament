/*
 *  happy.c
 *  written by Holmes Futrell
 *  use however you want
 */

#include "SDL.h"
#include "common.h"

#define NUM_HAPPY_FACES 100     /* number of faces to draw */
#define HAPPY_FACE_SIZE 32      /* width and height of happyface */

static SDL_Texture *texture = 0;    /* reference to texture holding happyface */

static struct
{
    float x, y;                 /* position of happyface */
    float xvel, yvel;           /* velocity of happyface */
} faces[NUM_HAPPY_FACES];

/*
    Sets initial positions and velocities of happyfaces
    units of velocity are pixels per millesecond
*/
void
initializeHappyFaces(SDL_Renderer *renderer)
{
    int i;
    int w;
    int h;
    SDL_RenderGetLogicalSize(renderer, &w, &h);

    for (i = 0; i < NUM_HAPPY_FACES; i++) {
        faces[i].x = randomFloat(0.0f, w - HAPPY_FACE_SIZE);
        faces[i].y = randomFloat(0.0f, h - HAPPY_FACE_SIZE);
        faces[i].xvel = randomFloat(-60.0f, 60.0f);
        faces[i].yvel = randomFloat(-60.0f, 60.0f);
    }
}

void
render(SDL_Renderer *renderer, double deltaTime)
{
    int i;
    SDL_Rect srcRect;
    SDL_Rect dstRect;
    int w;
    int h;

    SDL_RenderGetLogicalSize(renderer, &w, &h);

    /* setup boundaries for happyface bouncing */
    int maxx = w - HAPPY_FACE_SIZE;
    int maxy = h - HAPPY_FACE_SIZE;
    int minx = 0;
    int miny = 0;

    /* setup rects for drawing */
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = HAPPY_FACE_SIZE;
    srcRect.h = HAPPY_FACE_SIZE;
    dstRect.w = HAPPY_FACE_SIZE;
    dstRect.h = HAPPY_FACE_SIZE;

    /* fill background in with black */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    /*
       loop through all the happy faces:
       - update position
       - update velocity (if boundary is hit)
       - draw
     */
    for (i = 0; i < NUM_HAPPY_FACES; i++) {
        faces[i].x += faces[i].xvel * deltaTime;
        faces[i].y += faces[i].yvel * deltaTime;
        if (faces[i].x > maxx) {
            faces[i].x = maxx;
            faces[i].xvel = -faces[i].xvel;
        } else if (faces[i].y > maxy) {
            faces[i].y = maxy;
            faces[i].yvel = -faces[i].yvel;
        }
        if (faces[i].x < minx) {
            faces[i].x = minx;
            faces[i].xvel = -faces[i].xvel;
        } else if (faces[i].y < miny) {
            faces[i].y = miny;
            faces[i].yvel = -faces[i].yvel;
        }
        dstRect.x = faces[i].x;
        dstRect.y = faces[i].y;
        SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
    }
    /* update screen */
    SDL_RenderPresent(renderer);

}

/*
    loads the happyface graphic into a texture
*/
void
initializeTexture(SDL_Renderer *renderer)
{
    SDL_Surface *bmp_surface;
    /* load the bmp */
    bmp_surface = SDL_LoadBMP("icon.bmp");
    if (bmp_surface == NULL) {
        fatalError("could not load bmp");
    }
    /* set white to transparent on the happyface */
    SDL_SetColorKey(bmp_surface, 1,
                    SDL_MapRGB(bmp_surface->format, 255, 255, 255));

    /* convert RGBA surface to texture */
    texture = SDL_CreateTextureFromSurface(renderer, bmp_surface);
    if (texture == 0) {
        fatalError("could not create texture");
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    /* free up allocated memory */
    SDL_FreeSurface(bmp_surface);
}

int
main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int done;
    int width;
    int height;

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fatalError("Could not initialize SDL");
    }

    /* The specified window size doesn't matter - except for its aspect ratio,
     * which determines whether the window is in portrait or landscape on iOS
     * (if SDL_WINDOW_RESIZABLE isn't specified). */
    window = SDL_CreateWindow(NULL, 0, 0, 320, 480, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI);

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_GetWindowSize(window, &width, &height);
    SDL_RenderSetLogicalSize(renderer, width, height);

    initializeTexture(renderer);
    initializeHappyFaces(renderer);


    /* main loop */
    done = 0;
    while (!done) {
        SDL_Event event;
        double deltaTime = updateDeltaTime();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            }
        }

        render(renderer, deltaTime);
        SDL_Delay(1);
    }

    /* cleanup */
    SDL_DestroyTexture(texture);
    /* shutdown SDL */
    SDL_Quit();

    return 0;

}
