/*
 *  fireworks.c
 *  written by Holmes Futrell
 *  use however you want
 */

#include "SDL.h"
#include "SDL_opengles.h"
#include "common.h"
#include <math.h>
#include <time.h>

#define ACCEL 0.0001f           /* acceleration due to gravity, units in pixels per millesecond squared */
#define WIND_RESISTANCE 0.00005f        /* acceleration per unit velocity due to wind resistance */
#define MAX_PARTICLES 2000      /* maximum number of particles displayed at once */

static GLuint particleTextureID;        /* OpenGL particle texture id */
static SDL_bool pointSizeExtensionSupported;    /* is GL_OES_point_size_array supported ? */
static float pointSizeScale;
/*
    used to describe what type of particle a given struct particle is.
    emitter - this particle flies up, shooting off trail particles, then finally explodes into dust particles.
    trail   - shoots off, following emitter particle
    dust    - radiates outwards from emitter explosion
*/
enum particleType
{
    emitter = 0,
    trail,
    dust
};
/*
    struct particle is used to describe each particle displayed on screen
*/
struct particle
{
    GLfloat x;                  /* x position of particle */
    GLfloat y;                  /* y position of particle */
    GLubyte color[4];           /* rgba color of particle */
    GLfloat size;               /* size of particle in pixels */
    GLfloat xvel;               /* x velocity of particle in pixels per milesecond */
    GLfloat yvel;               /* y velocity of particle in pixels per millescond */
    int isActive;               /* if not active, then particle is overwritten */
    enum particleType type;     /* see enum particleType */
} particles[MAX_PARTICLES];     /* this array holds all our particles */

static int num_active_particles;        /* how many members of the particle array are actually being drawn / animated? */
static int screen_w, screen_h;

/* function declarations */
void spawnTrailFromEmitter(struct particle *emitter);
void spawnEmitterParticle(GLfloat x, GLfloat y);
void explodeEmitter(struct particle *emitter);
void initializeParticles(void);
void initializeTexture();
int nextPowerOfTwo(int x);
void drawParticles();
void stepParticles(double deltaTime);

/*  helper function (used in texture loading)
    returns next power of two greater than or equal to x
*/
int
nextPowerOfTwo(int x)
{
    int val = 1;
    while (val < x) {
        val *= 2;
    }
    return val;
}

/*
    steps each active particle by timestep deltaTime
*/
void
stepParticles(double deltaTime)
{
    float deltaMilliseconds = deltaTime * 1000;
    int i;
    struct particle *slot = particles;
    struct particle *curr = particles;
    for (i = 0; i < num_active_particles; i++) {
        /* is the particle actually active, or is it marked for deletion? */
        if (curr->isActive) {
            /* is the particle off the screen? */
            if (curr->y > screen_h)
                curr->isActive = 0;
            else if (curr->y < 0)
                curr->isActive = 0;
            if (curr->x > screen_w)
                curr->isActive = 0;
            else if (curr->x < 0)
                curr->isActive = 0;

            /* step velocity, then step position */
            curr->yvel += ACCEL * deltaMilliseconds;
            curr->xvel += 0.0f;
            curr->y += curr->yvel * deltaMilliseconds;
            curr->x += curr->xvel * deltaMilliseconds;

            /* particle behavior */
            if (curr->type == emitter) {
                /* if we're an emitter, spawn a trail */
                spawnTrailFromEmitter(curr);
                /* if we've reached our peak, explode */
                if (curr->yvel > 0.0) {
                    explodeEmitter(curr);
                }
            } else {
                float speed =
                    sqrt(curr->xvel * curr->xvel + curr->yvel * curr->yvel);
                /*      if wind resistance is not powerful enough to stop us completely,
                   then apply winde resistance, otherwise just stop us completely */
                if (WIND_RESISTANCE * deltaMilliseconds < speed) {
                    float normx = curr->xvel / speed;
                    float normy = curr->yvel / speed;
                    curr->xvel -=
                        normx * WIND_RESISTANCE * deltaMilliseconds;
                    curr->yvel -=
                        normy * WIND_RESISTANCE * deltaMilliseconds;
                } else {
                    curr->xvel = curr->yvel = 0;        /* stop particle */
                }

                if (curr->color[3] <= deltaMilliseconds * 0.1275f) {
                    /* if this next step will cause us to fade out completely
                       then just mark for deletion */
                    curr->isActive = 0;
                } else {
                    /* otherwise, let's fade a bit more */
                    curr->color[3] -= deltaMilliseconds * 0.1275f;
                }

                /* if we're a dust particle, shrink our size */
                if (curr->type == dust)
                    curr->size -= deltaMilliseconds * 0.010f;

            }

            /* if we're still active, pack ourselves in the array next
               to the last active guy (pack the array tightly) */
            if (curr->isActive)
                *(slot++) = *curr;
        }                       /* endif (curr->isActive) */
        curr++;
    }
    /* the number of active particles is computed as the difference between
       old number of active particles, where slot points, and the
       new size of the array, where particles points */
    num_active_particles = (int) (slot - particles);
}

/*
    This draws all the particles shown on screen
*/
void
drawParticles()
{

    /* draw the background */
    glClear(GL_COLOR_BUFFER_BIT);

    /* set up the position and color pointers */
    glVertexPointer(2, GL_FLOAT, sizeof(struct particle), particles);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(struct particle),
                   particles[0].color);

    if (pointSizeExtensionSupported) {
        /* pass in our array of point sizes */
        glPointSizePointerOES(GL_FLOAT, sizeof(struct particle),
                              &(particles[0].size));
    }

    /* draw our particles! */
    glDrawArrays(GL_POINTS, 0, num_active_particles);

}

/*
    This causes an emitter to explode in a circular bloom of dust particles
*/
void
explodeEmitter(struct particle *emitter)
{
    /* first off, we're done with this particle, so turn active off */
    emitter->isActive = 0;
    int i;
    for (i = 0; i < 200; i++) {

        if (num_active_particles >= MAX_PARTICLES)
            return;

        /* come up with a random angle and speed for new particle */
        float theta = randomFloat(0, 2.0f * 3.141592);
        float exponent = 3.0f;
        float speed = randomFloat(0.00, powf(0.17, exponent));
        speed = powf(speed, 1.0f / exponent);

        /* select the particle at the end of our array */
        struct particle *p = &particles[num_active_particles];

        /* set the particles properties */
        p->xvel = speed * cos(theta);
        p->yvel = speed * sin(theta);
        p->x = emitter->x + emitter->xvel;
        p->y = emitter->y + emitter->yvel;
        p->isActive = 1;
        p->type = dust;
        p->size = 15 * pointSizeScale;
        /* inherit emitter's color */
        p->color[0] = emitter->color[0];
        p->color[1] = emitter->color[1];
        p->color[2] = emitter->color[2];
        p->color[3] = 255;
        /* our array has expanded at the end */
        num_active_particles++;
    }

}

/*
    This spawns a trail particle from an emitter
*/
void
spawnTrailFromEmitter(struct particle *emitter)
{

    if (num_active_particles >= MAX_PARTICLES)
        return;

    /* select the particle at the slot at the end of our array */
    struct particle *p = &particles[num_active_particles];

    /* set position and velocity to roughly that of the emitter */
    p->x = emitter->x + randomFloat(-3.0, 3.0);
    p->y = emitter->y + emitter->size / 2.0f;
    p->xvel = emitter->xvel + randomFloat(-0.005, 0.005);
    p->yvel = emitter->yvel + 0.1;

    /* set the color to a random-ish orangy type color */
    p->color[0] = (0.8f + randomFloat(-0.1, 0.0)) * 255;
    p->color[1] = (0.4f + randomFloat(-0.1, 0.1)) * 255;
    p->color[2] = (0.0f + randomFloat(0.0, 0.2)) * 255;
    p->color[3] = (0.7f) * 255;

    /* set other attributes */
    p->size = 10 * pointSizeScale;
    p->type = trail;
    p->isActive = 1;

    /* our array has expanded at the end */
    num_active_particles++;

}

/*
    spawns a new emitter particle at the bottom of the screen
    destined for the point (x,y).
*/
void
spawnEmitterParticle(GLfloat x, GLfloat y)
{

    if (num_active_particles >= MAX_PARTICLES)
        return;

    /* find particle at endpoint of array */
    struct particle *p = &particles[num_active_particles];

    /* set the color randomly */
    switch (rand() % 4) {
    case 0:
        p->color[0] = 255;
        p->color[1] = 100;
        p->color[2] = 100;
        break;
    case 1:
        p->color[0] = 100;
        p->color[1] = 255;
        p->color[2] = 100;
        break;
    case 2:
        p->color[0] = 100;
        p->color[1] = 100;
        p->color[2] = 255;
        break;
    case 3:
        p->color[0] = 255;
        p->color[1] = 150;
        p->color[2] = 50;
        break;
    }
    p->color[3] = 255;
    /* set position to (x, screen_h) */
    p->x = x;
    p->y = screen_h;
    /* set velocity so that terminal point is (x,y) */
    p->xvel = 0;
    p->yvel = -sqrt(2 * ACCEL * (screen_h - y));
    /* set other attributes */
    p->size = 10 * pointSizeScale;
    p->type = emitter;
    p->isActive = 1;
    /* our array has expanded at the end */
    num_active_particles++;
}

/* just sets the endpoint of the particle array to element zero */
void
initializeParticles(void)
{
    num_active_particles = 0;
}

/*
    loads the particle texture
 */
void
initializeTexture()
{

    int bpp;                    /* texture bits per pixel */
    Uint32 Rmask, Gmask, Bmask, Amask;  /* masks for pixel format passed into OpenGL */
    SDL_Surface *bmp_surface;   /* the bmp is loaded here */
    SDL_Surface *bmp_surface_rgba8888;  /* this serves as a destination to convert the BMP
                                           to format passed into OpenGL */

    bmp_surface = SDL_LoadBMP("stroke.bmp");
    if (bmp_surface == NULL) {
        fatalError("could not load stroke.bmp");
    }

    /* Grab info about format that will be passed into OpenGL */
    SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ABGR8888, &bpp, &Rmask, &Gmask,
                               &Bmask, &Amask);
    /* Create surface that will hold pixels passed into OpenGL */
    bmp_surface_rgba8888 =
        SDL_CreateRGBSurface(0, bmp_surface->w, bmp_surface->h, bpp, Rmask,
                             Gmask, Bmask, Amask);
    /* Blit to this surface, effectively converting the format */
    SDL_BlitSurface(bmp_surface, NULL, bmp_surface_rgba8888, NULL);

    glGenTextures(1, &particleTextureID);
    glBindTexture(GL_TEXTURE_2D, particleTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 nextPowerOfTwo(bmp_surface->w),
                 nextPowerOfTwo(bmp_surface->h),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    /* this is where we actually pass in the pixel data */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmp_surface->w, bmp_surface->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, bmp_surface_rgba8888->pixels);

    /* free bmp surface and converted bmp surface */
    SDL_FreeSurface(bmp_surface);
    SDL_FreeSurface(bmp_surface_rgba8888);

}

int
main(int argc, char *argv[])
{
    SDL_Window *window;         /* main window */
    SDL_GLContext context;
    int drawableW, drawableH;
    int done;                   /* should we clean up and exit? */

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fatalError("Could not initialize SDL");
    }
    /* seed the random number generator */
    srand(time(NULL));
    /*
       request some OpenGL parameters
       that may speed drawing
     */
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    /* create main window and renderer */
    window = SDL_CreateWindow(NULL, 0, 0, 320, 480,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI);
    context = SDL_GL_CreateContext(window);

    /* The window size and drawable size may be different when highdpi is enabled,
     * due to the increased pixel density of the drawable. */
    SDL_GetWindowSize(window, &screen_w, &screen_h);
    SDL_GL_GetDrawableSize(window, &drawableW, &drawableH);

    /* In OpenGL, point sizes are always in pixels. We don't want them looking
     * tiny on a retina screen. */
    pointSizeScale = (float) drawableH / (float) screen_h;

    /* load the particle texture */
    initializeTexture();

    /*      check if GL_POINT_SIZE_ARRAY_OES is supported
       this is used to give each particle its own size
     */
    pointSizeExtensionSupported =
        SDL_GL_ExtensionSupported("GL_OES_point_size_array");

    /* set up some OpenGL state */
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, drawableW, drawableH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof((GLfloat) 0,
             (GLfloat) screen_w,
             (GLfloat) screen_h,
             (GLfloat) 0, 0.0, 1.0);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glEnable(GL_POINT_SPRITE_OES);
    glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, 1);

    if (pointSizeExtensionSupported) {
        /* we use this to set the sizes of all the particles */
        glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
    } else {
        /* if extension not available then all particles have size 10 */
        glPointSize(10 * pointSizeScale);
    }

    done = 0;
    /* enter main loop */
    while (!done) {
        SDL_Event event;
        double deltaTime = updateDeltaTime();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                spawnEmitterParticle(x, y);
            }
        }
        stepParticles(deltaTime);
        drawParticles();
        SDL_GL_SwapWindow(window);
        SDL_Delay(1);
    }

    /* delete textures */
    glDeleteTextures(1, &particleTextureID);
    /* shutdown SDL */
    SDL_Quit();

    return 0;
}
