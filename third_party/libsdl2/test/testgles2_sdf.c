/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "SDL_test_common.h"

#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__) || defined(__NACL__) \
    || defined(__WINDOWS__) || defined(__LINUX__)
#define HAVE_OPENGLES2
#endif

#ifdef HAVE_OPENGLES2

#include "SDL_opengles2.h"

typedef struct GLES2_Context
{
#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "../src/render/opengles2/SDL_gles2funcs.h"
#undef SDL_PROC
} GLES2_Context;


static SDL_Surface *g_surf_sdf = NULL;
GLenum g_texture;
GLenum g_texture_type = GL_TEXTURE_2D;
GLfloat g_verts[24];
typedef enum
{
    GLES2_ATTRIBUTE_POSITION = 0,
    GLES2_ATTRIBUTE_TEXCOORD = 1,
    GLES2_ATTRIBUTE_ANGLE = 2,
    GLES2_ATTRIBUTE_CENTER = 3,
} GLES2_Attribute;

typedef enum
{
    GLES2_UNIFORM_PROJECTION,
    GLES2_UNIFORM_TEXTURE,
    GLES2_UNIFORM_COLOR,
} GLES2_Uniform;


GLuint g_uniform_locations[16];



static SDLTest_CommonState *state;
static SDL_GLContext *context = NULL;
static int depth = 16;
static GLES2_Context ctx;

static int LoadContext(GLES2_Context * data)
{
#if SDL_VIDEO_DRIVER_UIKIT
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_ANDROID
#define __SDL_NOGETPROCADDR__
#elif SDL_VIDEO_DRIVER_PANDORA
#define __SDL_NOGETPROCADDR__
#endif

#if defined __SDL_NOGETPROCADDR__
#define SDL_PROC(ret,func,params) data->func=func;
#else
#define SDL_PROC(ret,func,params) \
    do { \
        data->func = SDL_GL_GetProcAddress(#func); \
        if ( ! data->func ) { \
            return SDL_SetError("Couldn't load GLES2 function %s: %s", #func, SDL_GetError()); \
        } \
    } while ( 0 );
#endif /* __SDL_NOGETPROCADDR__ */

#include "../src/render/opengles2/SDL_gles2funcs.h"
#undef SDL_PROC
    return 0;
}

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    int i;

    if (context != NULL) {
        for (i = 0; i < state->num_windows; i++) {
            if (context[i]) {
                SDL_GL_DeleteContext(context[i]);
            }
        }

        SDL_free(context);
    }

    SDLTest_CommonQuit(state);
    exit(rc);
}

#define GL_CHECK(x) \
        x; \
        { \
          GLenum glError = ctx.glGetError(); \
          if(glError != GL_NO_ERROR) { \
            SDL_Log("glGetError() = %i (0x%.8x) at line %i\n", glError, glError, __LINE__); \
            quit(1); \
          } \
        }


/* 
 * Create shader, load in source, compile, dump debug as necessary.
 *
 * shader: Pointer to return created shader ID.
 * source: Passed-in shader source code.
 * shader_type: Passed to GL, e.g. GL_VERTEX_SHADER.
 */
void 
process_shader(GLuint *shader, const char * source, GLint shader_type)
{
    GLint status = GL_FALSE;
    const char *shaders[1] = { NULL };
    char buffer[1024];
    GLsizei length;

    /* Create shader and load into GL. */
    *shader = GL_CHECK(ctx.glCreateShader(shader_type));

    shaders[0] = source;

    GL_CHECK(ctx.glShaderSource(*shader, 1, shaders, NULL));

    /* Clean up shader source. */
    shaders[0] = NULL;

    /* Try compiling the shader. */
    GL_CHECK(ctx.glCompileShader(*shader));
    GL_CHECK(ctx.glGetShaderiv(*shader, GL_COMPILE_STATUS, &status));

    /* Dump debug info (source and log) if compilation failed. */
    if(status != GL_TRUE) {
        ctx.glGetProgramInfoLog(*shader, sizeof(buffer), &length, &buffer[0]);
        buffer[length] = '\0';
        SDL_Log("Shader compilation failed: %s", buffer);fflush(stderr);
        quit(-1);
    }
}

/* Notes on a_angle:
   * It is a vector containing sin and cos for rotation matrix
   * To get correct rotation for most cases when a_angle is disabled cos
     value is decremented by 1.0 to get proper output with 0.0 which is
     default value
*/
static const Uint8 GLES2_VertexSrc_Default_[] = " \
    uniform mat4 u_projection; \
    attribute vec2 a_position; \
    attribute vec2 a_texCoord; \
    attribute vec2 a_angle; \
    attribute vec2 a_center; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        float s = a_angle[0]; \
        float c = a_angle[1] + 1.0; \
        mat2 rotationMatrix = mat2(c, -s, s, c); \
        vec2 position = rotationMatrix * (a_position - a_center) + a_center; \
        v_texCoord = a_texCoord; \
        gl_Position = u_projection * vec4(position, 0.0, 1.0);\
        gl_PointSize = 1.0; \
    } \
";

static const Uint8 GLES2_FragmentSrc_TextureABGRSrc_[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_color; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        gl_FragColor = texture2D(u_texture, v_texCoord); \
        gl_FragColor *= u_color; \
    } \
";

/* RGB to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureABGRSrc_SDF[] = " \
    #extension GL_OES_standard_derivatives : enable\n\
    \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_color; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
\
        float sigDist = abgr.a; \
        \
        float w = fwidth( sigDist );\
        float alpha = clamp(smoothstep(0.5 - w, 0.5 + w, sigDist), 0.0, 1.0); \
\
        gl_FragColor = vec4(abgr.rgb, abgr.a * alpha); \
        gl_FragColor.rgb *= gl_FragColor.a; \
        gl_FragColor *= u_color; \
    } \
";

/* RGB to ABGR conversion DEBUG */
static const char *GLES2_FragmentSrc_TextureABGRSrc_SDF_dbg = " \
    #extension GL_OES_standard_derivatives : enable\n\
    \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_color; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
\
        float a = abgr.a; \
        gl_FragColor = vec4(a, a, a, 1.0); \
    } \
";


static float g_val = 1.0f;
static int   g_use_SDF = 1;
static int   g_use_SDF_debug = 0;
static float g_angle = 0.0f;
static float matrix_mvp[4][4];




typedef struct shader_data
{
    GLuint shader_program, shader_frag, shader_vert;

    GLint attr_position;
    GLint attr_color, attr_mvp;

} shader_data;

static void
Render(unsigned int width, unsigned int height, shader_data* data)
{
    float *verts = g_verts;
    ctx.glViewport(0, 0, 640, 480);

    GL_CHECK(ctx.glClear(GL_COLOR_BUFFER_BIT));

    GL_CHECK(ctx.glUniformMatrix4fv(g_uniform_locations[GLES2_UNIFORM_PROJECTION], 1, GL_FALSE, (const float *)matrix_mvp));
    GL_CHECK(ctx.glUniform4f(g_uniform_locations[GLES2_UNIFORM_COLOR], 1.0f, 1.0f, 1.0f, 1.0f));

    GL_CHECK(ctx.glVertexAttribPointer(GLES2_ATTRIBUTE_ANGLE,    2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) (verts + 16)));
    GL_CHECK(ctx.glVertexAttribPointer(GLES2_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) (verts + 8)));
    GL_CHECK(ctx.glVertexAttribPointer(GLES2_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *) verts));

    GL_CHECK(ctx.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}


void renderCopy_angle(float degree_angle) 
{
    const float radian_angle = (float)(3.141592 * degree_angle) / 180.0;
    const GLfloat s = (GLfloat) SDL_sin(radian_angle);
    const GLfloat c = (GLfloat) SDL_cos(radian_angle) - 1.0f;
    GLfloat *verts = g_verts + 16;
    *(verts++) = s;
    *(verts++) = c;
    *(verts++) = s;
    *(verts++) = c;
    *(verts++) = s;
    *(verts++) = c;
    *(verts++) = s;
    *(verts++) = c;
}


void renderCopy_position(SDL_Rect *srcrect, SDL_Rect *dstrect) 
{
    GLfloat minx, miny, maxx, maxy;
    GLfloat minu, maxu, minv, maxv;
    GLfloat *verts = g_verts;

    minx = dstrect->x;
    miny = dstrect->y;
    maxx = dstrect->x + dstrect->w;
    maxy = dstrect->y + dstrect->h;

    minu = (GLfloat) srcrect->x / g_surf_sdf->w;
    maxu = (GLfloat) (srcrect->x + srcrect->w) / g_surf_sdf->w;
    minv = (GLfloat) srcrect->y / g_surf_sdf->h;
    maxv = (GLfloat) (srcrect->y + srcrect->h) / g_surf_sdf->h;

    *(verts++) = minx;
    *(verts++) = miny;
    *(verts++) = maxx;
    *(verts++) = miny;
    *(verts++) = minx;
    *(verts++) = maxy;
    *(verts++) = maxx;
    *(verts++) = maxy;

    *(verts++) = minu;
    *(verts++) = minv;
    *(verts++) = maxu;
    *(verts++) = minv;
    *(verts++) = minu;
    *(verts++) = maxv;
    *(verts++) = maxu;
    *(verts++) = maxv;
}

int done;
Uint32 frames;
shader_data *datas;

void loop()
{
    SDL_Event event;
    int i;
    int status;

    /* Check for events */
    ++frames;
    while (SDL_PollEvent(&event) && !done) {
        switch (event.type) {
        case SDL_KEYDOWN:
            {
                const int sym = event.key.keysym.sym;

                if (sym == SDLK_TAB) {
                    SDL_Log("Tab");


                }


                if (sym == SDLK_LEFT)  g_val -= 0.05;
                if (sym == SDLK_RIGHT) g_val += 0.05;
                if (sym == SDLK_UP)    g_angle -= 1;
                if (sym == SDLK_DOWN)  g_angle += 1;
 

                break;
            }

        case SDL_WINDOWEVENT:
            switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    for (i = 0; i < state->num_windows; ++i) {
                        if (event.window.windowID == SDL_GetWindowID(state->windows[i])) {
                            int w, h;
                            status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
                            if (status) {
                                SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
                                break;
                            }
                            /* Change view port to the new window dimensions */
                            SDL_GL_GetDrawableSize(state->windows[i], &w, &h);
                            ctx.glViewport(0, 0, w, h);
                            state->window_w = event.window.data1;
                            state->window_h = event.window.data2;
                            /* Update window content */
                            Render(event.window.data1, event.window.data2, &datas[i]);
                            SDL_GL_SwapWindow(state->windows[i]);
                            break;
                        }
                    }
                    break;
            }
        }
        SDLTest_CommonEvent(state, &event, &done);
    }


    matrix_mvp[3][0] = -1.0f;
    matrix_mvp[3][3] = 1.0f;

    matrix_mvp[0][0] = 2.0f / 640.0;
    matrix_mvp[1][1] = -2.0f / 480.0;
    matrix_mvp[3][1] = 1.0f;
    
    if (0)
    {
        float *f = (float *) matrix_mvp;
        SDL_Log("-----------------------------------");
        SDL_Log("[ %f, %f, %f, %f ]", *f++, *f++, *f++, *f++);
        SDL_Log("[ %f, %f, %f, %f ]", *f++, *f++, *f++, *f++);
        SDL_Log("[ %f, %f, %f, %f ]", *f++, *f++, *f++, *f++);
        SDL_Log("[ %f, %f, %f, %f ]", *f++, *f++, *f++, *f++);
        SDL_Log("-----------------------------------");
    }

    renderCopy_angle(g_angle);

    {
        int w, h;
        SDL_Rect rs, rd;

        SDL_GL_GetDrawableSize(state->windows[0], &w, &h);

        rs.x = 0; rs.y = 0; rs.w = g_surf_sdf->w; rs.h = g_surf_sdf->h;
        rd.w = g_surf_sdf->w * g_val; rd.h = g_surf_sdf->h * g_val;
        rd.x = (w - rd.w) / 2; rd.y = (h - rd.h) / 2;
        renderCopy_position(&rs, &rd);
    }
    

    if (!done) {
      for (i = 0; i < state->num_windows; ++i) {
          status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
          if (status) {
              SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());

              /* Continue for next window */
              continue;
          }
          Render(state->window_w, state->window_h, &datas[i]);
          SDL_GL_SwapWindow(state->windows[i]);
      }
    }
#ifdef __EMSCRIPTEN__
    else {
        emscripten_cancel_main_loop();
    }
#endif
}

int
main(int argc, char *argv[])
{
    int fsaa, accel;
    int value;
    int i;
    SDL_DisplayMode mode;
    Uint32 then, now;
    int status;
    shader_data *data;

    /* Initialize parameters */
    fsaa = 0;
    accel = 0;

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, SDL_INIT_VIDEO);
    if (!state) {
        return 1;
    }
    for (i = 1; i < argc;) {
        int consumed;

        consumed = SDLTest_CommonArg(state, i);
        if (consumed == 0) {
            if (SDL_strcasecmp(argv[i], "--fsaa") == 0) {
                ++fsaa;
                consumed = 1;
            } else if (SDL_strcasecmp(argv[i], "--accel") == 0) {
                ++accel;
                consumed = 1;
            } else if (SDL_strcasecmp(argv[i], "--zdepth") == 0) {
                i++;
                if (!argv[i]) {
                    consumed = -1;
                } else {
                    depth = SDL_atoi(argv[i]);
                    consumed = 1;
                }
            } else {
                consumed = -1;
            }
        }
        if (consumed < 0) {
            static const char *options[] = { "[--fsaa]", "[--accel]", "[--zdepth %d]", NULL };
            SDLTest_CommonLogUsage(state, argv[0], options);
            quit(1);
        }
        i += consumed;
    }

    /* Set OpenGL parameters */
    state->window_flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    state->gl_red_size = 5;
    state->gl_green_size = 5;
    state->gl_blue_size = 5;
    state->gl_depth_size = depth;
    state->gl_major_version = 2;
    state->gl_minor_version = 0;
    state->gl_profile_mask = SDL_GL_CONTEXT_PROFILE_ES;

    if (fsaa) {
        state->gl_multisamplebuffers=1;
        state->gl_multisamplesamples=fsaa;
    }
    if (accel) {
        state->gl_accelerated=1;
    }
    if (!SDLTest_CommonInit(state)) {
        quit(2);
        return 0;
    }

    context = (SDL_GLContext *)SDL_calloc(state->num_windows, sizeof(context));
    if (context == NULL) {
        SDL_Log("Out of memory!\n");
        quit(2);
    }
    
    /* Create OpenGL ES contexts */
    for (i = 0; i < state->num_windows; i++) {
        context[i] = SDL_GL_CreateContext(state->windows[i]);
        if (!context[i]) {
            SDL_Log("SDL_GL_CreateContext(): %s\n", SDL_GetError());
            quit(2);
        }
    }

    /* Important: call this *after* creating the context */
    if (LoadContext(&ctx) < 0) {
        SDL_Log("Could not load GLES2 functions\n");
        quit(2);
        return 0;
    }

    SDL_memset(matrix_mvp, 0, sizeof (matrix_mvp));
    
    {
        SDL_Surface *tmp;
        char *f;
        g_use_SDF = 1;
        g_use_SDF_debug = 0;

        if (g_use_SDF) {
            f = "testgles2_sdf_img_sdf.bmp";
        } else {
            f = "testgles2_sdf_img_normal.bmp";
        }
            
        SDL_Log("SDF is %s", g_use_SDF ? "enabled" : "disabled");

        /* Load SDF BMP image */
#if 1
        tmp = SDL_LoadBMP(f);
        if  (tmp == NULL) {
            SDL_Log("missing image file: %s", f);
            exit(-1);
        } else {
            SDL_Log("Load image file: %s", f);
        }

#else
        /* Generate SDF image using SDL_ttf */

        #include "SDL_ttf.h"
        char *font_file = "./font/DroidSansFallback.ttf";
        char *str = "Abcde";
        SDL_Color color = {  0, 0,0,  255};

        TTF_Init();
        TTF_Font *font = TTF_OpenFont(font_file, 72);

        if (font == NULL) {
            SDL_Log("Cannot open font %s", font_file);
        }

        TTF_SetFontSDF(font, g_use_SDF);
        SDL_Surface *tmp = TTF_RenderUTF8_Blended(font, str, color);

        SDL_Log("err: %s", SDL_GetError());
        if (tmp == NULL) {
            SDL_Log("can't render text");
            return -1;
        }

        SDL_SaveBMP(tmp, f);

        TTF_CloseFont(font);
        TTF_Quit();
#endif
        g_surf_sdf = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_ABGR8888, 0);

        SDL_SetSurfaceBlendMode(g_surf_sdf, SDL_BLENDMODE_BLEND);
    }


    if (state->render_flags & SDL_RENDERER_PRESENTVSYNC) {
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_GL_SetSwapInterval(0);
    }

    SDL_GetCurrentDisplayMode(0, &mode);
    SDL_Log("Screen bpp: %d\n", SDL_BITSPERPIXEL(mode.format));
    SDL_Log("\n");
    SDL_Log("Vendor     : %s\n", ctx.glGetString(GL_VENDOR));
    SDL_Log("Renderer   : %s\n", ctx.glGetString(GL_RENDERER));
    SDL_Log("Version    : %s\n", ctx.glGetString(GL_VERSION));
    SDL_Log("Extensions : %s\n", ctx.glGetString(GL_EXTENSIONS));
    SDL_Log("\n");

    status = SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_RED_SIZE: requested %d, got %d\n", 5, value);
    } else {
        SDL_Log( "Failed to get SDL_GL_RED_SIZE: %s\n",
                SDL_GetError());
    }
    status = SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_GREEN_SIZE: requested %d, got %d\n", 5, value);
    } else {
        SDL_Log( "Failed to get SDL_GL_GREEN_SIZE: %s\n",
                SDL_GetError());
    }
    status = SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_BLUE_SIZE: requested %d, got %d\n", 5, value);
    } else {
        SDL_Log( "Failed to get SDL_GL_BLUE_SIZE: %s\n",
                SDL_GetError());
    }
    status = SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
    if (!status) {
        SDL_Log("SDL_GL_DEPTH_SIZE: requested %d, got %d\n", depth, value);
    } else {
        SDL_Log( "Failed to get SDL_GL_DEPTH_SIZE: %s\n",
                SDL_GetError());
    }
    if (fsaa) {
        status = SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &value);
        if (!status) {
            SDL_Log("SDL_GL_MULTISAMPLEBUFFERS: requested 1, got %d\n", value);
        } else {
            SDL_Log( "Failed to get SDL_GL_MULTISAMPLEBUFFERS: %s\n",
                    SDL_GetError());
        }
        status = SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &value);
        if (!status) {
            SDL_Log("SDL_GL_MULTISAMPLESAMPLES: requested %d, got %d\n", fsaa,
                   value);
        } else {
            SDL_Log( "Failed to get SDL_GL_MULTISAMPLESAMPLES: %s\n",
                    SDL_GetError());
        }
    }
    if (accel) {
        status = SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &value);
        if (!status) {
            SDL_Log("SDL_GL_ACCELERATED_VISUAL: requested 1, got %d\n", value);
        } else {
            SDL_Log( "Failed to get SDL_GL_ACCELERATED_VISUAL: %s\n",
                    SDL_GetError());
        }
    }

    datas = (shader_data *)SDL_calloc(state->num_windows, sizeof(shader_data));

    /* Set rendering settings for each context */
    for (i = 0; i < state->num_windows; ++i) {

        int w, h;
        status = SDL_GL_MakeCurrent(state->windows[i], context[i]);
        if (status) {
            SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());

            /* Continue for next window */
            continue;
        }

        {
            int format = GL_RGBA;
            int type = GL_UNSIGNED_BYTE;

            GL_CHECK(ctx.glGenTextures(1, &g_texture));

            ctx.glActiveTexture(GL_TEXTURE0);
            ctx.glPixelStorei(GL_PACK_ALIGNMENT, 1);
            ctx.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            ctx.glBindTexture(g_texture_type, g_texture);

            ctx.glTexParameteri(g_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            ctx.glTexParameteri(g_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            ctx.glTexParameteri(g_texture_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            ctx.glTexParameteri(g_texture_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            GL_CHECK(ctx.glTexImage2D(g_texture_type, 0, format, g_surf_sdf->w, g_surf_sdf->h, 0, format, type, NULL));
            GL_CHECK(ctx.glTexSubImage2D(g_texture_type, 0, 0 /* xoffset */, 0 /* yoffset */, g_surf_sdf->w, g_surf_sdf->h, format, type, g_surf_sdf->pixels));
        }


        SDL_GL_GetDrawableSize(state->windows[i], &w, &h);
        ctx.glViewport(0, 0, w, h);

        data = &datas[i];

        /* Shader Initialization */
        process_shader(&data->shader_vert, GLES2_VertexSrc_Default_, GL_VERTEX_SHADER);

        if (g_use_SDF) {
            if (g_use_SDF_debug == 0) {
                process_shader(&data->shader_frag, GLES2_FragmentSrc_TextureABGRSrc_SDF, GL_FRAGMENT_SHADER);
            } else {
                process_shader(&data->shader_frag, GLES2_FragmentSrc_TextureABGRSrc_SDF_dbg, GL_FRAGMENT_SHADER);
            }
        } else {
            process_shader(&data->shader_frag, GLES2_FragmentSrc_TextureABGRSrc_, GL_FRAGMENT_SHADER);
        }

        /* Create shader_program (ready to attach shaders) */
        data->shader_program = GL_CHECK(ctx.glCreateProgram());

        /* Attach shaders and link shader_program */
        GL_CHECK(ctx.glAttachShader(data->shader_program, data->shader_vert));
        GL_CHECK(ctx.glAttachShader(data->shader_program, data->shader_frag));
        GL_CHECK(ctx.glLinkProgram(data->shader_program));

        ctx.glBindAttribLocation(data->shader_program, GLES2_ATTRIBUTE_POSITION, "a_position");
        ctx.glBindAttribLocation(data->shader_program, GLES2_ATTRIBUTE_TEXCOORD, "a_texCoord");
        ctx.glBindAttribLocation(data->shader_program, GLES2_ATTRIBUTE_ANGLE, "a_angle");
        ctx.glBindAttribLocation(data->shader_program, GLES2_ATTRIBUTE_CENTER, "a_center");

        /* Predetermine locations of uniform variables */
        g_uniform_locations[GLES2_UNIFORM_PROJECTION] = ctx.glGetUniformLocation(data->shader_program, "u_projection");
        g_uniform_locations[GLES2_UNIFORM_TEXTURE] = ctx.glGetUniformLocation(data->shader_program, "u_texture");
        g_uniform_locations[GLES2_UNIFORM_COLOR] = ctx.glGetUniformLocation(data->shader_program, "u_color");

        GL_CHECK(ctx.glUseProgram(data->shader_program));

        ctx.glEnableVertexAttribArray((GLenum) GLES2_ATTRIBUTE_ANGLE);
        ctx.glDisableVertexAttribArray((GLenum) GLES2_ATTRIBUTE_CENTER);
        ctx.glEnableVertexAttribArray(GLES2_ATTRIBUTE_POSITION);
        ctx.glEnableVertexAttribArray((GLenum) GLES2_ATTRIBUTE_TEXCOORD);


    ctx.glUniform1i(g_uniform_locations[GLES2_UNIFORM_TEXTURE], 0);  /* always texture unit 0. */
    ctx.glActiveTexture(GL_TEXTURE0);
    ctx.glBindTexture(g_texture_type, g_texture);
     GL_CHECK(ctx.glClearColor(1, 1, 1, 1));

    // SDL_BLENDMODE_BLEND
    GL_CHECK(ctx.glEnable(GL_BLEND));
    ctx.glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    ctx.glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);


    }

    /* Main render loop */
    frames = 0;
    then = SDL_GetTicks();
    done = 0;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!done) {
        loop();
    }
#endif

    /* Print out some timing information */
    now = SDL_GetTicks();
    if (now > then) {
        SDL_Log("%2.2f frames per second\n",
               ((double) frames * 1000) / (now - then));
    }
#if !defined(__ANDROID__) && !defined(__NACL__)  
    quit(0);
#endif    
    return 0;
}

#else /* HAVE_OPENGLES2 */

int
main(int argc, char *argv[])
{
    SDL_Log("No OpenGL ES support on this system\n");
    return 1;
}

#endif /* HAVE_OPENGLES2 */

/* vi: set ts=4 sw=4 expandtab: */
