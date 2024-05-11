/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_OGL_ES2 && !SDL_RENDER_DISABLED

#include "SDL_assert.h"
#include "SDL_hints.h"
#include "SDL_opengles2.h"
#include "../SDL_sysrender.h"
#include "../../video/SDL_blit.h"
#include "SDL_shaders_gles2.h"

/* !!! FIXME: Emscripten makes these into WebGL calls, and WebGL doesn't offer
   !!! FIXME:  client-side arrays (without an Emscripten compatibility hack,
   !!! FIXME:  at least), but the current VBO code here is dramatically
   !!! FIXME:  slower on actual iOS devices, even though the iOS Simulator
   !!! FIXME:  is okay. Some time after 2.0.4 ships, we should revisit this,
   !!! FIXME:  fix the performance bottleneck, and make everything use VBOs.
*/
#ifdef __EMSCRIPTEN__
#define SDL_GLES2_USE_VBOS 1
#else
#define SDL_GLES2_USE_VBOS 0
#endif

/* To prevent unnecessary window recreation,
 * these should match the defaults selected in SDL_GL_ResetAttributes 
 */
#define RENDERER_CONTEXT_MAJOR 2
#define RENDERER_CONTEXT_MINOR 0

/* Used to re-create the window with OpenGL ES capability */
extern int SDL_RecreateWindow(SDL_Window * window, Uint32 flags);

/*************************************************************************************************
 * Bootstrap data                                                                                *
 *************************************************************************************************/

static SDL_Renderer *GLES2_CreateRenderer(SDL_Window *window, Uint32 flags);

SDL_RenderDriver GLES2_RenderDriver = {
    GLES2_CreateRenderer,
    {
        "opengles2",
        (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE),
        4,
        {
        SDL_PIXELFORMAT_ARGB8888,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_PIXELFORMAT_RGB888,
        SDL_PIXELFORMAT_BGR888
        },
        0,
        0
    }
};

/*************************************************************************************************
 * Context structures                                                                            *
 *************************************************************************************************/

typedef struct GLES2_FBOList GLES2_FBOList;

struct GLES2_FBOList
{
   Uint32 w, h;
   GLuint FBO;
   GLES2_FBOList *next;
};

typedef struct GLES2_TextureData
{
    GLenum texture;
    GLenum texture_type;
    GLenum pixel_format;
    GLenum pixel_type;
    void *pixel_data;
    int pitch;
    /* YUV texture support */
    SDL_bool yuv;
    SDL_bool nv12;
    GLenum texture_v;
    GLenum texture_u;
    GLES2_FBOList *fbo;
} GLES2_TextureData;

typedef struct GLES2_ShaderCacheEntry
{
    GLuint id;
    GLES2_ShaderType type;
    const GLES2_ShaderInstance *instance;
    int references;
    Uint8 modulation_r, modulation_g, modulation_b, modulation_a;
    struct GLES2_ShaderCacheEntry *prev;
    struct GLES2_ShaderCacheEntry *next;
} GLES2_ShaderCacheEntry;

typedef struct GLES2_ShaderCache
{
    int count;
    GLES2_ShaderCacheEntry *head;
} GLES2_ShaderCache;

typedef struct GLES2_ProgramCacheEntry
{
    GLuint id;
    GLES2_ShaderCacheEntry *vertex_shader;
    GLES2_ShaderCacheEntry *fragment_shader;
    GLuint uniform_locations[16];
    Uint8 color_r, color_g, color_b, color_a;
    Uint8 modulation_r, modulation_g, modulation_b, modulation_a;
    GLfloat projection[4][4];
    struct GLES2_ProgramCacheEntry *prev;
    struct GLES2_ProgramCacheEntry *next;
} GLES2_ProgramCacheEntry;

typedef struct GLES2_ProgramCache
{
    int count;
    GLES2_ProgramCacheEntry *head;
    GLES2_ProgramCacheEntry *tail;
} GLES2_ProgramCache;

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
    GLES2_UNIFORM_MODULATION,
    GLES2_UNIFORM_COLOR,
    GLES2_UNIFORM_TEXTURE_U,
    GLES2_UNIFORM_TEXTURE_V
} GLES2_Uniform;

typedef enum
{
    GLES2_IMAGESOURCE_SOLID,
    GLES2_IMAGESOURCE_TEXTURE_ABGR,
    GLES2_IMAGESOURCE_TEXTURE_ARGB,
    GLES2_IMAGESOURCE_TEXTURE_RGB,
    GLES2_IMAGESOURCE_TEXTURE_BGR,
    GLES2_IMAGESOURCE_TEXTURE_YUV,
    GLES2_IMAGESOURCE_TEXTURE_NV12,
    GLES2_IMAGESOURCE_TEXTURE_NV21,
    GLES2_IMAGESOURCE_TEXTURE_EXTERNAL_OES
} GLES2_ImageSource;

typedef struct GLES2_DriverContext
{
    SDL_GLContext *context;

    SDL_bool debug_enabled;

    struct {
        SDL_BlendMode blendMode;
        SDL_bool tex_coords;
    } current;

#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "SDL_gles2funcs.h"
#undef SDL_PROC
    GLES2_FBOList *framebuffers;
    GLuint window_framebuffer;

    int shader_format_count;
    GLenum *shader_formats;
    GLES2_ShaderCache shader_cache;
    GLES2_ProgramCache program_cache;
    GLES2_ProgramCacheEntry *current_program;
    Uint8 clear_r, clear_g, clear_b, clear_a;

#if SDL_GLES2_USE_VBOS
    GLuint vertex_buffers[4];
    GLsizeiptr vertex_buffer_size[4];
#endif
} GLES2_DriverContext;

#define GLES2_MAX_CACHED_PROGRAMS 8


SDL_FORCE_INLINE const char*
GL_TranslateError (GLenum error)
{
#define GL_ERROR_TRANSLATE(e) case e: return #e;
    switch (error) {
    GL_ERROR_TRANSLATE(GL_INVALID_ENUM)
    GL_ERROR_TRANSLATE(GL_INVALID_VALUE)
    GL_ERROR_TRANSLATE(GL_INVALID_OPERATION)
    GL_ERROR_TRANSLATE(GL_OUT_OF_MEMORY)
    GL_ERROR_TRANSLATE(GL_NO_ERROR)
    default:
        return "UNKNOWN";
}
#undef GL_ERROR_TRANSLATE
}

SDL_FORCE_INLINE void
GL_ClearErrors(SDL_Renderer *renderer)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *) renderer->driverdata;

    if (!data->debug_enabled) {
        return;
    }
    while (data->glGetError() != GL_NO_ERROR) {
        continue;
    }
}

SDL_FORCE_INLINE int
GL_CheckAllErrors (const char *prefix, SDL_Renderer *renderer, const char *file, int line, const char *function)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *) renderer->driverdata;
    int ret = 0;

    if (!data->debug_enabled) {
        return 0;
    }
    /* check gl errors (can return multiple errors) */
    for (;;) {
        GLenum error = data->glGetError();
        if (error != GL_NO_ERROR) {
            if (prefix == NULL || prefix[0] == '\0') {
                prefix = "generic";
            }
            SDL_SetError("%s: %s (%d): %s %s (0x%X)", prefix, file, line, function, GL_TranslateError(error), error);
            ret = -1;
        } else {
            break;
        }
    }
    return ret;
}

#if 0
#define GL_CheckError(prefix, renderer)
#else
#define GL_CheckError(prefix, renderer) GL_CheckAllErrors(prefix, renderer, SDL_FILE, SDL_LINE, SDL_FUNCTION)
#endif


/*************************************************************************************************
 * Renderer state APIs                                                                           *
 *************************************************************************************************/

static int GLES2_ActivateRenderer(SDL_Renderer *renderer);
static void GLES2_WindowEvent(SDL_Renderer * renderer,
                              const SDL_WindowEvent *event);
static int GLES2_UpdateViewport(SDL_Renderer * renderer);
static void GLES2_DestroyRenderer(SDL_Renderer *renderer);
static int GLES2_SetOrthographicProjection(SDL_Renderer *renderer);


static SDL_GLContext SDL_CurrentContext = NULL;

static int GLES2_LoadFunctions(GLES2_DriverContext * data)
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

#include "SDL_gles2funcs.h"
#undef SDL_PROC
    return 0;
}

static GLES2_FBOList *
GLES2_GetFBO(GLES2_DriverContext *data, Uint32 w, Uint32 h)
{
   GLES2_FBOList *result = data->framebuffers;
   while ((result) && ((result->w != w) || (result->h != h)) ) {
       result = result->next;
   }
   if (result == NULL) {
       result = SDL_malloc(sizeof(GLES2_FBOList));
       result->w = w;
       result->h = h;
       data->glGenFramebuffers(1, &result->FBO);
       result->next = data->framebuffers;
       data->framebuffers = result;
   }
   return result;
}

static int
GLES2_ActivateRenderer(SDL_Renderer * renderer)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;

    if (SDL_CurrentContext != data->context) {
        /* Null out the current program to ensure we set it again */
        data->current_program = NULL;

        if (SDL_GL_MakeCurrent(renderer->window, data->context) < 0) {
            return -1;
        }
        SDL_CurrentContext = data->context;

        GLES2_UpdateViewport(renderer);
    }

    GL_ClearErrors(renderer);

    return 0;
}

static void
GLES2_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;

    if (event->event == SDL_WINDOWEVENT_SIZE_CHANGED ||
        event->event == SDL_WINDOWEVENT_SHOWN ||
        event->event == SDL_WINDOWEVENT_HIDDEN) {
        /* Rebind the context to the window area */
        SDL_CurrentContext = NULL;
    }

    if (event->event == SDL_WINDOWEVENT_MINIMIZED) {
        /* According to Apple documentation, we need to finish drawing NOW! */
        data->glFinish();
    }
}

static int
GLES2_GetOutputSize(SDL_Renderer * renderer, int *w, int *h)
{
    SDL_GL_GetDrawableSize(renderer->window, w, h);
    return 0;
}

static GLenum GetBlendFunc(SDL_BlendFactor factor)
{
    switch (factor) {
    case SDL_BLENDFACTOR_ZERO:
        return GL_ZERO;
    case SDL_BLENDFACTOR_ONE:
        return GL_ONE;
    case SDL_BLENDFACTOR_SRC_COLOR:
        return GL_SRC_COLOR;
    case SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    case SDL_BLENDFACTOR_SRC_ALPHA:
        return GL_SRC_ALPHA;
    case SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case SDL_BLENDFACTOR_DST_COLOR:
        return GL_DST_COLOR;
    case SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR:
        return GL_ONE_MINUS_DST_COLOR;
    case SDL_BLENDFACTOR_DST_ALPHA:
        return GL_DST_ALPHA;
    case SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA:
        return GL_ONE_MINUS_DST_ALPHA;
    default:
        return GL_INVALID_ENUM;
    }
}

static GLenum GetBlendEquation(SDL_BlendOperation operation)
{
    switch (operation) {
    case SDL_BLENDOPERATION_ADD:
        return GL_FUNC_ADD;
    case SDL_BLENDOPERATION_SUBTRACT:
        return GL_FUNC_SUBTRACT;
    case SDL_BLENDOPERATION_REV_SUBTRACT:
        return GL_FUNC_REVERSE_SUBTRACT;
    default:
        return GL_INVALID_ENUM;
    }
}

static SDL_bool
GLES2_SupportsBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode)
{
    SDL_BlendFactor srcColorFactor = SDL_GetBlendModeSrcColorFactor(blendMode);
    SDL_BlendFactor srcAlphaFactor = SDL_GetBlendModeSrcAlphaFactor(blendMode);
    SDL_BlendOperation colorOperation = SDL_GetBlendModeColorOperation(blendMode);
    SDL_BlendFactor dstColorFactor = SDL_GetBlendModeDstColorFactor(blendMode);
    SDL_BlendFactor dstAlphaFactor = SDL_GetBlendModeDstAlphaFactor(blendMode);
    SDL_BlendOperation alphaOperation = SDL_GetBlendModeAlphaOperation(blendMode);

    if (GetBlendFunc(srcColorFactor) == GL_INVALID_ENUM ||
        GetBlendFunc(srcAlphaFactor) == GL_INVALID_ENUM ||
        GetBlendEquation(colorOperation) == GL_INVALID_ENUM ||
        GetBlendFunc(dstColorFactor) == GL_INVALID_ENUM ||
        GetBlendFunc(dstAlphaFactor) == GL_INVALID_ENUM ||
        GetBlendEquation(alphaOperation) == GL_INVALID_ENUM) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static int
GLES2_UpdateViewport(SDL_Renderer * renderer)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;

    if (SDL_CurrentContext != data->context) {
        /* We'll update the viewport after we rebind the context */
        return 0;
    }

    if (renderer->target) {
        data->glViewport(renderer->viewport.x, renderer->viewport.y,
                         renderer->viewport.w, renderer->viewport.h);
    } else {
        int w, h;

        SDL_GL_GetDrawableSize(renderer->window, &w, &h);
        data->glViewport(renderer->viewport.x, (h - renderer->viewport.y - renderer->viewport.h),
                         renderer->viewport.w, renderer->viewport.h);
    }

    if (data->current_program) {
        GLES2_SetOrthographicProjection(renderer);
    }
    return GL_CheckError("", renderer);
}

static int
GLES2_UpdateClipRect(SDL_Renderer * renderer)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;

    if (SDL_CurrentContext != data->context) {
        /* We'll update the clip rect after we rebind the context */
        return 0;
    }

    if (renderer->clipping_enabled) {
        const SDL_Rect *rect = &renderer->clip_rect;
        data->glEnable(GL_SCISSOR_TEST);
        if (renderer->target) {
            data->glScissor(renderer->viewport.x + rect->x, renderer->viewport.y + rect->y, rect->w, rect->h);
        } else {
            int w, h;

            SDL_GL_GetDrawableSize(renderer->window, &w, &h);
            data->glScissor(renderer->viewport.x + rect->x, h - renderer->viewport.y - rect->y - rect->h, rect->w, rect->h);
        }
    } else {
        data->glDisable(GL_SCISSOR_TEST);
    }
    return 0;
}

static void
GLES2_DestroyRenderer(SDL_Renderer *renderer)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;

    /* Deallocate everything */
    if (data) {
        GLES2_ActivateRenderer(renderer);

        {
            GLES2_ShaderCacheEntry *entry;
            GLES2_ShaderCacheEntry *next;
            entry = data->shader_cache.head;
            while (entry) {
                data->glDeleteShader(entry->id);
                next = entry->next;
                SDL_free(entry);
                entry = next;
            }
        }
        {
            GLES2_ProgramCacheEntry *entry;
            GLES2_ProgramCacheEntry *next;
            entry = data->program_cache.head;
            while (entry) {
                data->glDeleteProgram(entry->id);
                next = entry->next;
                SDL_free(entry);
                entry = next;
            }
        }
        if (data->context) {
            while (data->framebuffers) {
                GLES2_FBOList *nextnode = data->framebuffers->next;
                data->glDeleteFramebuffers(1, &data->framebuffers->FBO);
                GL_CheckError("", renderer);
                SDL_free(data->framebuffers);
                data->framebuffers = nextnode;
            }
            SDL_GL_DeleteContext(data->context);
        }
        SDL_free(data->shader_formats);
        SDL_free(data);
    }
    SDL_free(renderer);
}

/*************************************************************************************************
 * Texture APIs                                                                                  *
 *************************************************************************************************/

static int GLES2_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture);
static int GLES2_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                               const void *pixels, int pitch);
static int GLES2_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
                               const SDL_Rect * rect,
                               const Uint8 *Yplane, int Ypitch,
                               const Uint8 *Uplane, int Upitch,
                               const Uint8 *Vplane, int Vpitch);
static int GLES2_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                             void **pixels, int *pitch);
static void GLES2_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture);
static int GLES2_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture);
static void GLES2_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture);

static GLenum
GetScaleQuality(void)
{
    const char *hint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);

    if (!hint || *hint == '0' || SDL_strcasecmp(hint, "nearest") == 0) {
        return GL_NEAREST;
    } else {
        return GL_LINEAR;
    }
}

static int
GLES2_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    GLES2_DriverContext *renderdata = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_TextureData *data;
    GLenum format;
    GLenum type;
    GLenum scaleMode;

    GLES2_ActivateRenderer(renderer);

    /* Determine the corresponding GLES texture format params */
    switch (texture->format)
    {
    case SDL_PIXELFORMAT_ARGB8888:
    case SDL_PIXELFORMAT_ABGR8888:
    case SDL_PIXELFORMAT_RGB888:
    case SDL_PIXELFORMAT_BGR888:
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        break;
    case SDL_PIXELFORMAT_IYUV:
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        format = GL_LUMINANCE;
        type = GL_UNSIGNED_BYTE;
        break;
#ifdef GL_TEXTURE_EXTERNAL_OES
    case SDL_PIXELFORMAT_EXTERNAL_OES:
        format = GL_NONE;
        type = GL_NONE;
        break;
#endif
    default:
        return SDL_SetError("Texture format not supported");
    }

    if (texture->format == SDL_PIXELFORMAT_EXTERNAL_OES &&
        texture->access != SDL_TEXTUREACCESS_STATIC) {
        return SDL_SetError("Unsupported texture access for SDL_PIXELFORMAT_EXTERNAL_OES");
    }

    /* Allocate a texture struct */
    data = (GLES2_TextureData *)SDL_calloc(1, sizeof(GLES2_TextureData));
    if (!data) {
        return SDL_OutOfMemory();
    }
    data->texture = 0;
#ifdef GL_TEXTURE_EXTERNAL_OES
    data->texture_type = (texture->format == SDL_PIXELFORMAT_EXTERNAL_OES) ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D;
#else
    data->texture_type = GL_TEXTURE_2D;
#endif
    data->pixel_format = format;
    data->pixel_type = type;
    data->yuv = ((texture->format == SDL_PIXELFORMAT_IYUV) || (texture->format == SDL_PIXELFORMAT_YV12));
    data->nv12 = ((texture->format == SDL_PIXELFORMAT_NV12) || (texture->format == SDL_PIXELFORMAT_NV21));
    data->texture_u = 0;
    data->texture_v = 0;
    scaleMode = GetScaleQuality();

    /* Allocate a blob for image renderdata */
    if (texture->access == SDL_TEXTUREACCESS_STREAMING) {
        size_t size;
        data->pitch = texture->w * SDL_BYTESPERPIXEL(texture->format);
        size = texture->h * data->pitch;
        if (data->yuv) {
            /* Need to add size for the U and V planes */
            size += 2 * ((texture->h + 1) / 2) * ((data->pitch + 1) / 2);
        }
        if (data->nv12) {
            /* Need to add size for the U/V plane */
            size += 2 * ((texture->h + 1) / 2) * ((data->pitch + 1) / 2);
        }
        data->pixel_data = SDL_calloc(1, size);
        if (!data->pixel_data) {
            SDL_free(data);
            return SDL_OutOfMemory();
        }
    }

    /* Allocate the texture */
    GL_CheckError("", renderer);

    if (data->yuv) {
        renderdata->glGenTextures(1, &data->texture_v);
        if (GL_CheckError("glGenTexures()", renderer) < 0) {
            return -1;
        }
        renderdata->glActiveTexture(GL_TEXTURE2);
        renderdata->glBindTexture(data->texture_type, data->texture_v);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_MIN_FILTER, scaleMode);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_MAG_FILTER, scaleMode);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        renderdata->glTexImage2D(data->texture_type, 0, format, (texture->w + 1) / 2, (texture->h + 1) / 2, 0, format, type, NULL);

        renderdata->glGenTextures(1, &data->texture_u);
        if (GL_CheckError("glGenTexures()", renderer) < 0) {
            return -1;
        }
        renderdata->glActiveTexture(GL_TEXTURE1);
        renderdata->glBindTexture(data->texture_type, data->texture_u);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_MIN_FILTER, scaleMode);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_MAG_FILTER, scaleMode);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        renderdata->glTexImage2D(data->texture_type, 0, format, (texture->w + 1) / 2, (texture->h + 1) / 2, 0, format, type, NULL);
        if (GL_CheckError("glTexImage2D()", renderer) < 0) {
            return -1;
        }
    }

    if (data->nv12) {
        renderdata->glGenTextures(1, &data->texture_u);
        if (GL_CheckError("glGenTexures()", renderer) < 0) {
            return -1;
        }
        renderdata->glActiveTexture(GL_TEXTURE1);
        renderdata->glBindTexture(data->texture_type, data->texture_u);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_MIN_FILTER, scaleMode);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_MAG_FILTER, scaleMode);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        renderdata->glTexImage2D(data->texture_type, 0, GL_LUMINANCE_ALPHA, (texture->w + 1) / 2, (texture->h + 1) / 2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, NULL);
        if (GL_CheckError("glTexImage2D()", renderer) < 0) {
            return -1;
        }
    }

    renderdata->glGenTextures(1, &data->texture);
    if (GL_CheckError("glGenTexures()", renderer) < 0) {
        return -1;
    }
    texture->driverdata = data;
    renderdata->glActiveTexture(GL_TEXTURE0);
    renderdata->glBindTexture(data->texture_type, data->texture);
    renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_MIN_FILTER, scaleMode);
    renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_MAG_FILTER, scaleMode);
    renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    renderdata->glTexParameteri(data->texture_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (texture->format != SDL_PIXELFORMAT_EXTERNAL_OES) {
        renderdata->glTexImage2D(data->texture_type, 0, format, texture->w, texture->h, 0, format, type, NULL);
        if (GL_CheckError("glTexImage2D()", renderer) < 0) {
            return -1;
        }
    }

    if (texture->access == SDL_TEXTUREACCESS_TARGET) {
       data->fbo = GLES2_GetFBO(renderer->driverdata, texture->w, texture->h);
    } else {
       data->fbo = NULL;
    }

    return GL_CheckError("", renderer);
}

static int
GLES2_TexSubImage2D(GLES2_DriverContext *data, GLenum target, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels, GLint pitch, GLint bpp)
{
    Uint8 *blob = NULL;
    Uint8 *src;
    int src_pitch;
    int y;

    /* Reformat the texture data into a tightly packed array */
    src_pitch = width * bpp;
    src = (Uint8 *)pixels;
    if (pitch != src_pitch) {
        blob = (Uint8 *)SDL_malloc(src_pitch * height);
        if (!blob) {
            return SDL_OutOfMemory();
        }
        src = blob;
        for (y = 0; y < height; ++y)
        {
            SDL_memcpy(src, pixels, src_pitch);
            src += src_pitch;
            pixels = (Uint8 *)pixels + pitch;
        }
        src = blob;
    }

    data->glTexSubImage2D(target, 0, xoffset, yoffset, width, height, format, type, src);
    if (blob) {
        SDL_free(blob);
    }
    return 0;
}

static int
GLES2_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                    const void *pixels, int pitch)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_TextureData *tdata = (GLES2_TextureData *)texture->driverdata;

    GLES2_ActivateRenderer(renderer);

    /* Bail out if we're supposed to update an empty rectangle */
    if (rect->w <= 0 || rect->h <= 0) {
        return 0;
    }

    /* Create a texture subimage with the supplied data */
    data->glBindTexture(tdata->texture_type, tdata->texture);
    GLES2_TexSubImage2D(data, tdata->texture_type,
                    rect->x,
                    rect->y,
                    rect->w,
                    rect->h,
                    tdata->pixel_format,
                    tdata->pixel_type,
                    pixels, pitch, SDL_BYTESPERPIXEL(texture->format));

    if (tdata->yuv) {
        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + rect->h * pitch);
        if (texture->format == SDL_PIXELFORMAT_YV12) {
            data->glBindTexture(tdata->texture_type, tdata->texture_v);
        } else {
            data->glBindTexture(tdata->texture_type, tdata->texture_u);
        }
        GLES2_TexSubImage2D(data, tdata->texture_type,
                rect->x / 2,
                rect->y / 2,
                (rect->w + 1) / 2,
                (rect->h + 1) / 2,
                tdata->pixel_format,
                tdata->pixel_type,
                pixels, (pitch + 1) / 2, 1);


        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + ((rect->h + 1) / 2) * ((pitch + 1)/2));
        if (texture->format == SDL_PIXELFORMAT_YV12) {
            data->glBindTexture(tdata->texture_type, tdata->texture_u);
        } else {
            data->glBindTexture(tdata->texture_type, tdata->texture_v);
        }
        GLES2_TexSubImage2D(data, tdata->texture_type,
                rect->x / 2,
                rect->y / 2,
                (rect->w + 1) / 2,
                (rect->h + 1) / 2,
                tdata->pixel_format,
                tdata->pixel_type,
                pixels, (pitch + 1) / 2, 1);
    }

    if (tdata->nv12) {
        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + rect->h * pitch);
        data->glBindTexture(tdata->texture_type, tdata->texture_u);
        GLES2_TexSubImage2D(data, tdata->texture_type,
                rect->x / 2,
                rect->y / 2,
                (rect->w + 1) / 2,
                (rect->h + 1) / 2,
                GL_LUMINANCE_ALPHA,
                GL_UNSIGNED_BYTE,
                pixels, 2 * ((pitch + 1) / 2), 2);
    }

    return GL_CheckError("glTexSubImage2D()", renderer);
}

static int
GLES2_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
                    const SDL_Rect * rect,
                    const Uint8 *Yplane, int Ypitch,
                    const Uint8 *Uplane, int Upitch,
                    const Uint8 *Vplane, int Vpitch)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_TextureData *tdata = (GLES2_TextureData *)texture->driverdata;

    GLES2_ActivateRenderer(renderer);

    /* Bail out if we're supposed to update an empty rectangle */
    if (rect->w <= 0 || rect->h <= 0) {
        return 0;
    }

    data->glBindTexture(tdata->texture_type, tdata->texture_v);
    GLES2_TexSubImage2D(data, tdata->texture_type,
                    rect->x / 2,
                    rect->y / 2,
                    (rect->w + 1) / 2,
                    (rect->h + 1) / 2,
                    tdata->pixel_format,
                    tdata->pixel_type,
                    Vplane, Vpitch, 1);

    data->glBindTexture(tdata->texture_type, tdata->texture_u);
    GLES2_TexSubImage2D(data, tdata->texture_type,
                    rect->x / 2,
                    rect->y / 2,
                    (rect->w + 1) / 2,
                    (rect->h + 1) / 2,
                    tdata->pixel_format,
                    tdata->pixel_type,
                    Uplane, Upitch, 1);

    data->glBindTexture(tdata->texture_type, tdata->texture);
    GLES2_TexSubImage2D(data, tdata->texture_type,
                    rect->x,
                    rect->y,
                    rect->w,
                    rect->h,
                    tdata->pixel_format,
                    tdata->pixel_type,
                    Yplane, Ypitch, 1);

    return GL_CheckError("glTexSubImage2D()", renderer);
}

static int
GLES2_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                  void **pixels, int *pitch)
{
    GLES2_TextureData *tdata = (GLES2_TextureData *)texture->driverdata;

    /* Retrieve the buffer/pitch for the specified region */
    *pixels = (Uint8 *)tdata->pixel_data +
              (tdata->pitch * rect->y) +
              (rect->x * SDL_BYTESPERPIXEL(texture->format));
    *pitch = tdata->pitch;

    return 0;
}

static void
GLES2_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    GLES2_TextureData *tdata = (GLES2_TextureData *)texture->driverdata;
    SDL_Rect rect;

    /* We do whole texture updates, at least for now */
    rect.x = 0;
    rect.y = 0;
    rect.w = texture->w;
    rect.h = texture->h;
    GLES2_UpdateTexture(renderer, texture, &rect, tdata->pixel_data, tdata->pitch);
}

static int
GLES2_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *) renderer->driverdata;
    GLES2_TextureData *texturedata = NULL;
    GLenum status;

    if (texture == NULL) {
        data->glBindFramebuffer(GL_FRAMEBUFFER, data->window_framebuffer);
    } else {
        texturedata = (GLES2_TextureData *) texture->driverdata;
        data->glBindFramebuffer(GL_FRAMEBUFFER, texturedata->fbo->FBO);
        /* TODO: check if texture pixel format allows this operation */
        data->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texturedata->texture_type, texturedata->texture, 0);
        /* Check FBO status */
        status = data->glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            return SDL_SetError("glFramebufferTexture2D() failed");
        }
    }
    return 0;
}

static void
GLES2_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_TextureData *tdata = (GLES2_TextureData *)texture->driverdata;

    GLES2_ActivateRenderer(renderer);

    /* Destroy the texture */
    if (tdata) {
        data->glDeleteTextures(1, &tdata->texture);
        if (tdata->texture_v) {
            data->glDeleteTextures(1, &tdata->texture_v);
        }
        if (tdata->texture_u) {
            data->glDeleteTextures(1, &tdata->texture_u);
        }
        SDL_free(tdata->pixel_data);
        SDL_free(tdata);
        texture->driverdata = NULL;
    }
}

/*************************************************************************************************
 * Shader management functions                                                                   *
 *************************************************************************************************/

static GLES2_ShaderCacheEntry *GLES2_CacheShader(SDL_Renderer *renderer, GLES2_ShaderType type);
static void GLES2_EvictShader(SDL_Renderer *renderer, GLES2_ShaderCacheEntry *entry);
static GLES2_ProgramCacheEntry *GLES2_CacheProgram(SDL_Renderer *renderer,
                                                   GLES2_ShaderCacheEntry *vertex,
                                                   GLES2_ShaderCacheEntry *fragment);
static int GLES2_SelectProgram(SDL_Renderer *renderer, GLES2_ImageSource source, int w, int h);

static GLES2_ProgramCacheEntry *
GLES2_CacheProgram(SDL_Renderer *renderer, GLES2_ShaderCacheEntry *vertex,
                   GLES2_ShaderCacheEntry *fragment)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_ProgramCacheEntry *entry;
    GLES2_ShaderCacheEntry *shaderEntry;
    GLint linkSuccessful;

    /* Check if we've already cached this program */
    entry = data->program_cache.head;
    while (entry) {
        if (entry->vertex_shader == vertex && entry->fragment_shader == fragment) {
            break;
        }
        entry = entry->next;
    }
    if (entry) {
        if (data->program_cache.head != entry) {
            if (entry->next) {
                entry->next->prev = entry->prev;
            }
            if (entry->prev) {
                entry->prev->next = entry->next;
            }
            entry->prev = NULL;
            entry->next = data->program_cache.head;
            data->program_cache.head->prev = entry;
            data->program_cache.head = entry;
        }
        return entry;
    }

    /* Create a program cache entry */
    entry = (GLES2_ProgramCacheEntry *)SDL_calloc(1, sizeof(GLES2_ProgramCacheEntry));
    if (!entry) {
        SDL_OutOfMemory();
        return NULL;
    }
    entry->vertex_shader = vertex;
    entry->fragment_shader = fragment;

    /* Create the program and link it */
    entry->id = data->glCreateProgram();
    data->glAttachShader(entry->id, vertex->id);
    data->glAttachShader(entry->id, fragment->id);
    data->glBindAttribLocation(entry->id, GLES2_ATTRIBUTE_POSITION, "a_position");
    data->glBindAttribLocation(entry->id, GLES2_ATTRIBUTE_TEXCOORD, "a_texCoord");
    data->glBindAttribLocation(entry->id, GLES2_ATTRIBUTE_ANGLE, "a_angle");
    data->glBindAttribLocation(entry->id, GLES2_ATTRIBUTE_CENTER, "a_center");
    data->glLinkProgram(entry->id);
    data->glGetProgramiv(entry->id, GL_LINK_STATUS, &linkSuccessful);
    if (!linkSuccessful) {
        data->glDeleteProgram(entry->id);
        SDL_free(entry);
        SDL_SetError("Failed to link shader program");
        return NULL;
    }

    /* Predetermine locations of uniform variables */
    entry->uniform_locations[GLES2_UNIFORM_PROJECTION] =
        data->glGetUniformLocation(entry->id, "u_projection");
    entry->uniform_locations[GLES2_UNIFORM_TEXTURE_V] =
        data->glGetUniformLocation(entry->id, "u_texture_v");
    entry->uniform_locations[GLES2_UNIFORM_TEXTURE_U] =
        data->glGetUniformLocation(entry->id, "u_texture_u");
    entry->uniform_locations[GLES2_UNIFORM_TEXTURE] =
        data->glGetUniformLocation(entry->id, "u_texture");
    entry->uniform_locations[GLES2_UNIFORM_MODULATION] =
        data->glGetUniformLocation(entry->id, "u_modulation");
    entry->uniform_locations[GLES2_UNIFORM_COLOR] =
        data->glGetUniformLocation(entry->id, "u_color");

    entry->modulation_r = entry->modulation_g = entry->modulation_b = entry->modulation_a = 255;
    entry->color_r = entry->color_g = entry->color_b = entry->color_a = 255;

    data->glUseProgram(entry->id);
    data->glUniform1i(entry->uniform_locations[GLES2_UNIFORM_TEXTURE_V], 2);  /* always texture unit 2. */
    data->glUniform1i(entry->uniform_locations[GLES2_UNIFORM_TEXTURE_U], 1);  /* always texture unit 1. */
    data->glUniform1i(entry->uniform_locations[GLES2_UNIFORM_TEXTURE], 0);  /* always texture unit 0. */
    data->glUniformMatrix4fv(entry->uniform_locations[GLES2_UNIFORM_PROJECTION], 1, GL_FALSE, (GLfloat *)entry->projection);
    data->glUniform4f(entry->uniform_locations[GLES2_UNIFORM_MODULATION], 1.0f, 1.0f, 1.0f, 1.0f);
    data->glUniform4f(entry->uniform_locations[GLES2_UNIFORM_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

    /* Cache the linked program */
    if (data->program_cache.head) {
        entry->next = data->program_cache.head;
        data->program_cache.head->prev = entry;
    } else {
        data->program_cache.tail = entry;
    }
    data->program_cache.head = entry;
    ++data->program_cache.count;

    /* Increment the refcount of the shaders we're using */
    ++vertex->references;
    ++fragment->references;

    /* Evict the last entry from the cache if we exceed the limit */
    if (data->program_cache.count > GLES2_MAX_CACHED_PROGRAMS) {
        shaderEntry = data->program_cache.tail->vertex_shader;
        if (--shaderEntry->references <= 0) {
            GLES2_EvictShader(renderer, shaderEntry);
        }
        shaderEntry = data->program_cache.tail->fragment_shader;
        if (--shaderEntry->references <= 0) {
            GLES2_EvictShader(renderer, shaderEntry);
        }
        data->glDeleteProgram(data->program_cache.tail->id);
        data->program_cache.tail = data->program_cache.tail->prev;
        SDL_free(data->program_cache.tail->next);
        data->program_cache.tail->next = NULL;
        --data->program_cache.count;
    }
    return entry;
}

static GLES2_ShaderCacheEntry *
GLES2_CacheShader(SDL_Renderer *renderer, GLES2_ShaderType type)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    const GLES2_Shader *shader;
    const GLES2_ShaderInstance *instance = NULL;
    GLES2_ShaderCacheEntry *entry = NULL;
    GLint compileSuccessful = GL_FALSE;
    int i, j;

    /* Find the corresponding shader */
    shader = GLES2_GetShader(type);
    if (!shader) {
        SDL_SetError("No shader matching the requested characteristics was found");
        return NULL;
    }

    /* Find a matching shader instance that's supported on this hardware */
    for (i = 0; i < shader->instance_count && !instance; ++i) {
        for (j = 0; j < data->shader_format_count && !instance; ++j) {
            if (!shader->instances[i]) {
                continue;
            }
            if (shader->instances[i]->format != data->shader_formats[j]) {
                continue;
            }
            instance = shader->instances[i];
        }
    }
    if (!instance) {
        SDL_SetError("The specified shader cannot be loaded on the current platform");
        return NULL;
    }

    /* Check if we've already cached this shader */
    entry = data->shader_cache.head;
    while (entry) {
        if (entry->instance == instance) {
            break;
        }
        entry = entry->next;
    }
    if (entry) {
        return entry;
    }

    /* Create a shader cache entry */
    entry = (GLES2_ShaderCacheEntry *)SDL_calloc(1, sizeof(GLES2_ShaderCacheEntry));
    if (!entry) {
        SDL_OutOfMemory();
        return NULL;
    }
    entry->type = type;
    entry->instance = instance;

    /* Compile or load the selected shader instance */
    entry->id = data->glCreateShader(instance->type);
    if (instance->format == (GLenum)-1) {
        data->glShaderSource(entry->id, 1, (const char **)(char *)&instance->data, NULL);
        data->glCompileShader(entry->id);
        data->glGetShaderiv(entry->id, GL_COMPILE_STATUS, &compileSuccessful);
    } else {
        data->glShaderBinary(1, &entry->id, instance->format, instance->data, instance->length);
        compileSuccessful = GL_TRUE;
    }
    if (!compileSuccessful) {
        char *info = NULL;
        int length = 0;

        data->glGetShaderiv(entry->id, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            info = SDL_stack_alloc(char, length);
            if (info) {
                data->glGetShaderInfoLog(entry->id, length, &length, info);
            }
        }
        if (info) {
            SDL_SetError("Failed to load the shader: %s", info);
            SDL_stack_free(info);
        } else {
            SDL_SetError("Failed to load the shader");
        }
        data->glDeleteShader(entry->id);
        SDL_free(entry);
        return NULL;
    }

    /* Link the shader entry in at the front of the cache */
    if (data->shader_cache.head) {
        entry->next = data->shader_cache.head;
        data->shader_cache.head->prev = entry;
    }
    data->shader_cache.head = entry;
    ++data->shader_cache.count;
    return entry;
}

static void
GLES2_EvictShader(SDL_Renderer *renderer, GLES2_ShaderCacheEntry *entry)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;

    /* Unlink the shader from the cache */
    if (entry->next) {
        entry->next->prev = entry->prev;
    }
    if (entry->prev) {
        entry->prev->next = entry->next;
    }
    if (data->shader_cache.head == entry) {
        data->shader_cache.head = entry->next;
    }
    --data->shader_cache.count;

    /* Deallocate the shader */
    data->glDeleteShader(entry->id);
    SDL_free(entry);
}

static int
GLES2_SelectProgram(SDL_Renderer *renderer, GLES2_ImageSource source, int w, int h)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_ShaderCacheEntry *vertex = NULL;
    GLES2_ShaderCacheEntry *fragment = NULL;
    GLES2_ShaderType vtype, ftype;
    GLES2_ProgramCacheEntry *program;

    /* Select an appropriate shader pair for the specified modes */
    vtype = GLES2_SHADER_VERTEX_DEFAULT;
    switch (source) {
    case GLES2_IMAGESOURCE_SOLID:
        ftype = GLES2_SHADER_FRAGMENT_SOLID_SRC;
        break;
    case GLES2_IMAGESOURCE_TEXTURE_ABGR:
        ftype = GLES2_SHADER_FRAGMENT_TEXTURE_ABGR_SRC;
        break;
    case GLES2_IMAGESOURCE_TEXTURE_ARGB:
        ftype = GLES2_SHADER_FRAGMENT_TEXTURE_ARGB_SRC;
        break;
    case GLES2_IMAGESOURCE_TEXTURE_RGB:
        ftype = GLES2_SHADER_FRAGMENT_TEXTURE_RGB_SRC;
        break;
    case GLES2_IMAGESOURCE_TEXTURE_BGR:
        ftype = GLES2_SHADER_FRAGMENT_TEXTURE_BGR_SRC;
        break;
    case GLES2_IMAGESOURCE_TEXTURE_YUV:
        switch (SDL_GetYUVConversionModeForResolution(w, h)) {
        case SDL_YUV_CONVERSION_JPEG:
            ftype = GLES2_SHADER_FRAGMENT_TEXTURE_YUV_JPEG_SRC;
            break;
        case SDL_YUV_CONVERSION_BT601:
            ftype = GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT601_SRC;
            break;
        case SDL_YUV_CONVERSION_BT709:
            ftype = GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT709_SRC;
            break;
        default:
            SDL_SetError("Unsupported YUV conversion mode: %d\n", SDL_GetYUVConversionModeForResolution(w, h));
            goto fault;
        }
        break;
    case GLES2_IMAGESOURCE_TEXTURE_NV12:
        switch (SDL_GetYUVConversionModeForResolution(w, h)) {
        case SDL_YUV_CONVERSION_JPEG:
            ftype = GLES2_SHADER_FRAGMENT_TEXTURE_NV12_JPEG_SRC;
            break;
        case SDL_YUV_CONVERSION_BT601:
            ftype = GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT601_SRC;
            break;
        case SDL_YUV_CONVERSION_BT709:
            ftype = GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT709_SRC;
            break;
        default:
            SDL_SetError("Unsupported YUV conversion mode: %d\n", SDL_GetYUVConversionModeForResolution(w, h));
            goto fault;
        }
        break;
    case GLES2_IMAGESOURCE_TEXTURE_NV21:
        switch (SDL_GetYUVConversionModeForResolution(w, h)) {
        case SDL_YUV_CONVERSION_JPEG:
            ftype = GLES2_SHADER_FRAGMENT_TEXTURE_NV21_JPEG_SRC;
            break;
        case SDL_YUV_CONVERSION_BT601:
            ftype = GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT601_SRC;
            break;
        case SDL_YUV_CONVERSION_BT709:
            ftype = GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT709_SRC;
            break;
        default:
            SDL_SetError("Unsupported YUV conversion mode: %d\n", SDL_GetYUVConversionModeForResolution(w, h));
            goto fault;
        }
        break;
    case GLES2_IMAGESOURCE_TEXTURE_EXTERNAL_OES:
        ftype = GLES2_SHADER_FRAGMENT_TEXTURE_EXTERNAL_OES_SRC;
        break;
    default:
        goto fault;
    }

    /* Load the requested shaders */
    vertex = GLES2_CacheShader(renderer, vtype);
    if (!vertex) {
        goto fault;
    }
    fragment = GLES2_CacheShader(renderer, ftype);
    if (!fragment) {
        goto fault;
    }

    /* Check if we need to change programs at all */
    if (data->current_program &&
        data->current_program->vertex_shader == vertex &&
        data->current_program->fragment_shader == fragment) {
        return 0;
    }

    /* Generate a matching program */
    program = GLES2_CacheProgram(renderer, vertex, fragment);
    if (!program) {
        goto fault;
    }

    /* Select that program in OpenGL */
    data->glUseProgram(program->id);

    /* Set the current program */
    data->current_program = program;

    /* Activate an orthographic projection */
    if (GLES2_SetOrthographicProjection(renderer) < 0) {
        goto fault;
    }

    /* Clean up and return */
    return 0;
fault:
    if (vertex && vertex->references <= 0) {
        GLES2_EvictShader(renderer, vertex);
    }
    if (fragment && fragment->references <= 0) {
        GLES2_EvictShader(renderer, fragment);
    }
    data->current_program = NULL;
    return -1;
}

static int
GLES2_SetOrthographicProjection(SDL_Renderer *renderer)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLfloat projection[4][4];

    if (!renderer->viewport.w || !renderer->viewport.h) {
        return 0;
    }

    /* Prepare an orthographic projection */
    projection[0][0] = 2.0f / renderer->viewport.w;
    projection[0][1] = 0.0f;
    projection[0][2] = 0.0f;
    projection[0][3] = 0.0f;
    projection[1][0] = 0.0f;
    if (renderer->target) {
        projection[1][1] = 2.0f / renderer->viewport.h;
    } else {
        projection[1][1] = -2.0f / renderer->viewport.h;
    }
    projection[1][2] = 0.0f;
    projection[1][3] = 0.0f;
    projection[2][0] = 0.0f;
    projection[2][1] = 0.0f;
    projection[2][2] = 0.0f;
    projection[2][3] = 0.0f;
    projection[3][0] = -1.0f;
    if (renderer->target) {
        projection[3][1] = -1.0f;
    } else {
        projection[3][1] = 1.0f;
    }
    projection[3][2] = 0.0f;
    projection[3][3] = 1.0f;

    /* Set the projection matrix */
    if (SDL_memcmp(data->current_program->projection, projection, sizeof (projection)) != 0) {
        const GLuint locProjection = data->current_program->uniform_locations[GLES2_UNIFORM_PROJECTION];
        data->glUniformMatrix4fv(locProjection, 1, GL_FALSE, (GLfloat *)projection);
        SDL_memcpy(data->current_program->projection, projection, sizeof (projection));
    }

    return 0;
}

/*************************************************************************************************
 * Rendering functions                                                                           *
 *************************************************************************************************/

static const float inv255f = 1.0f / 255.0f;

static int GLES2_RenderClear(SDL_Renderer *renderer);
static int GLES2_RenderDrawPoints(SDL_Renderer *renderer, const SDL_FPoint *points, int count);
static int GLES2_RenderDrawLines(SDL_Renderer *renderer, const SDL_FPoint *points, int count);
static int GLES2_RenderFillRects(SDL_Renderer *renderer, const SDL_FRect *rects, int count);
static int GLES2_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect,
                            const SDL_FRect *dstrect);
static int GLES2_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                         const SDL_Rect * srcrect, const SDL_FRect * dstrect,
                         const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip);
static int GLES2_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                    Uint32 pixel_format, void * pixels, int pitch);
static void GLES2_RenderPresent(SDL_Renderer *renderer);

static SDL_bool
CompareColors(Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1,
              Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2)
{
    Uint32 Pixel1, Pixel2;
    RGBA8888_FROM_RGBA(Pixel1, r1, g1, b1, a1);
    RGBA8888_FROM_RGBA(Pixel2, r2, g2, b2, a2);
    return (Pixel1 == Pixel2);
}

static int
GLES2_RenderClear(SDL_Renderer * renderer)
{
    Uint8 r, g, b, a;

    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;

    GLES2_ActivateRenderer(renderer);

    if (!CompareColors(data->clear_r, data->clear_g, data->clear_b, data->clear_a,
                        renderer->r, renderer->g, renderer->b, renderer->a)) {

       /* Select the color to clear with */
       g = renderer->g;
       a = renderer->a;
   
       if (renderer->target &&
            (renderer->target->format == SDL_PIXELFORMAT_ARGB8888 ||
             renderer->target->format == SDL_PIXELFORMAT_RGB888)) {
           r = renderer->b;
           b = renderer->r;
        } else {
           r = renderer->r;
           b = renderer->b;
        }

        data->glClearColor((GLfloat) r * inv255f,
                     (GLfloat) g * inv255f,
                     (GLfloat) b * inv255f,
                     (GLfloat) a * inv255f);
        data->clear_r = renderer->r;
        data->clear_g = renderer->g;
        data->clear_b = renderer->b;
        data->clear_a = renderer->a;
    }

    if (renderer->clipping_enabled) {
        data->glDisable(GL_SCISSOR_TEST);
    }

    data->glClear(GL_COLOR_BUFFER_BIT);

    if (renderer->clipping_enabled) {
        data->glEnable(GL_SCISSOR_TEST);
    }

    return 0;
}

static void
GLES2_SetBlendMode(GLES2_DriverContext *data, SDL_BlendMode blendMode)
{
    if (blendMode != data->current.blendMode) {
        if (blendMode == SDL_BLENDMODE_NONE) {
            data->glDisable(GL_BLEND);
        } else {
            data->glEnable(GL_BLEND);
            data->glBlendFuncSeparate(GetBlendFunc(SDL_GetBlendModeSrcColorFactor(blendMode)),
                                      GetBlendFunc(SDL_GetBlendModeDstColorFactor(blendMode)),
                                      GetBlendFunc(SDL_GetBlendModeSrcAlphaFactor(blendMode)),
                                      GetBlendFunc(SDL_GetBlendModeDstAlphaFactor(blendMode)));
            data->glBlendEquationSeparate(GetBlendEquation(SDL_GetBlendModeColorOperation(blendMode)),
                                          GetBlendEquation(SDL_GetBlendModeAlphaOperation(blendMode)));
        }
        data->current.blendMode = blendMode;
    }
}

static void
GLES2_SetTexCoords(GLES2_DriverContext * data, SDL_bool enabled)
{
    if (enabled != data->current.tex_coords) {
        if (enabled) {
            data->glEnableVertexAttribArray(GLES2_ATTRIBUTE_TEXCOORD);
        } else {
            data->glDisableVertexAttribArray(GLES2_ATTRIBUTE_TEXCOORD);
        }
        data->current.tex_coords = enabled;
    }
}

static int
GLES2_SetDrawingState(SDL_Renderer * renderer)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_ProgramCacheEntry *program;
    Uint8 r, g, b, a;

    GLES2_ActivateRenderer(renderer);

    GLES2_SetBlendMode(data, renderer->blendMode);

    GLES2_SetTexCoords(data, SDL_FALSE);

    /* Activate an appropriate shader and set the projection matrix */
    if (GLES2_SelectProgram(renderer, GLES2_IMAGESOURCE_SOLID, 0, 0) < 0) {
        return -1;
    }

    /* Select the color to draw with */
    g = renderer->g;
    a = renderer->a;

    if (renderer->target &&
         (renderer->target->format == SDL_PIXELFORMAT_ARGB8888 ||
         renderer->target->format == SDL_PIXELFORMAT_RGB888)) {
        r = renderer->b;
        b = renderer->r;
     } else {
        r = renderer->r;
        b = renderer->b;
     }

    program = data->current_program;
    if (!CompareColors(program->color_r, program->color_g, program->color_b, program->color_a, r, g, b, a)) {
        /* Select the color to draw with */
        data->glUniform4f(program->uniform_locations[GLES2_UNIFORM_COLOR], r * inv255f, g * inv255f, b * inv255f, a * inv255f);
        program->color_r = r;
        program->color_g = g;
        program->color_b = b;
        program->color_a = a;
    }

    return 0;
}

static int
GLES2_UpdateVertexBuffer(SDL_Renderer *renderer, GLES2_Attribute attr,
                         const void *vertexData, size_t dataSizeInBytes)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;

#if !SDL_GLES2_USE_VBOS
    data->glVertexAttribPointer(attr, attr == GLES2_ATTRIBUTE_ANGLE ? 1 : 2, GL_FLOAT, GL_FALSE, 0, vertexData);
#else
    if (!data->vertex_buffers[attr]) {
        data->glGenBuffers(1, &data->vertex_buffers[attr]);
    }

    data->glBindBuffer(GL_ARRAY_BUFFER, data->vertex_buffers[attr]);

    if (data->vertex_buffer_size[attr] < dataSizeInBytes) {
        data->glBufferData(GL_ARRAY_BUFFER, dataSizeInBytes, vertexData, GL_STREAM_DRAW);
        data->vertex_buffer_size[attr] = dataSizeInBytes;
    } else {
        data->glBufferSubData(GL_ARRAY_BUFFER, 0, dataSizeInBytes, vertexData);
    }

    data->glVertexAttribPointer(attr, attr == GLES2_ATTRIBUTE_ANGLE ? 1 : 2, GL_FLOAT, GL_FALSE, 0, 0);
#endif

    return 0;
}

static int
GLES2_RenderDrawPoints(SDL_Renderer *renderer, const SDL_FPoint *points, int count)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLfloat *vertices;
    int idx;

    if (GLES2_SetDrawingState(renderer) < 0) {
        return -1;
    }

    /* Emit the specified vertices as points */
    vertices = SDL_stack_alloc(GLfloat, count * 2);
    for (idx = 0; idx < count; ++idx) {
        GLfloat x = points[idx].x + 0.5f;
        GLfloat y = points[idx].y + 0.5f;

        vertices[idx * 2] = x;
        vertices[(idx * 2) + 1] = y;
    }
    /*data->glVertexAttribPointer(GLES2_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);*/
    GLES2_UpdateVertexBuffer(renderer, GLES2_ATTRIBUTE_POSITION, vertices, count * 2 * sizeof(GLfloat));
    data->glDrawArrays(GL_POINTS, 0, count);
    SDL_stack_free(vertices);
    return 0;
}

static int
GLES2_RenderDrawLines(SDL_Renderer *renderer, const SDL_FPoint *points, int count)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLfloat *vertices;
    int idx;

    if (GLES2_SetDrawingState(renderer) < 0) {
        return -1;
    }

    /* Emit a line strip including the specified vertices */
    vertices = SDL_stack_alloc(GLfloat, count * 2);
    for (idx = 0; idx < count; ++idx) {
        GLfloat x = points[idx].x + 0.5f;
        GLfloat y = points[idx].y + 0.5f;

        vertices[idx * 2] = x;
        vertices[(idx * 2) + 1] = y;
    }
    /*data->glVertexAttribPointer(GLES2_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);*/
    GLES2_UpdateVertexBuffer(renderer, GLES2_ATTRIBUTE_POSITION, vertices, count * 2 * sizeof(GLfloat));
    data->glDrawArrays(GL_LINE_STRIP, 0, count);

    /* We need to close the endpoint of the line */
    if (count == 2 ||
        points[0].x != points[count-1].x || points[0].y != points[count-1].y) {
        data->glDrawArrays(GL_POINTS, count-1, 1);
    }
    SDL_stack_free(vertices);

    return GL_CheckError("", renderer);
}

static int
GLES2_RenderFillRects(SDL_Renderer *renderer, const SDL_FRect *rects, int count)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLfloat vertices[8];
    int idx;

    if (GLES2_SetDrawingState(renderer) < 0) {
        return -1;
    }

    /* Emit a line loop for each rectangle */
    for (idx = 0; idx < count; ++idx) {
        const SDL_FRect *rect = &rects[idx];

        GLfloat xMin = rect->x;
        GLfloat xMax = (rect->x + rect->w);
        GLfloat yMin = rect->y;
        GLfloat yMax = (rect->y + rect->h);

        vertices[0] = xMin;
        vertices[1] = yMin;
        vertices[2] = xMax;
        vertices[3] = yMin;
        vertices[4] = xMin;
        vertices[5] = yMax;
        vertices[6] = xMax;
        vertices[7] = yMax;
        /*data->glVertexAttribPointer(GLES2_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);*/
        GLES2_UpdateVertexBuffer(renderer, GLES2_ATTRIBUTE_POSITION, vertices, 8 * sizeof(GLfloat));
        data->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    return GL_CheckError("", renderer);
}

static int
GLES2_SetupCopy(SDL_Renderer *renderer, SDL_Texture *texture)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_TextureData *tdata = (GLES2_TextureData *)texture->driverdata;
    GLES2_ImageSource sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;
    GLES2_ProgramCacheEntry *program;
    Uint8 r, g, b, a;

    /* Activate an appropriate shader and set the projection matrix */
    if (renderer->target) {
        /* Check if we need to do color mapping between the source and render target textures */
        if (renderer->target->format != texture->format) {
            switch (texture->format) {
            case SDL_PIXELFORMAT_ARGB8888:
                switch (renderer->target->format) {
                case SDL_PIXELFORMAT_ABGR8888:
                case SDL_PIXELFORMAT_BGR888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                    break;
                case SDL_PIXELFORMAT_RGB888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;
                    break;
                }
                break;
            case SDL_PIXELFORMAT_ABGR8888:
                switch (renderer->target->format) {
                case SDL_PIXELFORMAT_ARGB8888:
                case SDL_PIXELFORMAT_RGB888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                    break;
                case SDL_PIXELFORMAT_BGR888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;
                    break;
                }
                break;
            case SDL_PIXELFORMAT_RGB888:
                switch (renderer->target->format) {
                case SDL_PIXELFORMAT_ABGR8888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                    break;
                case SDL_PIXELFORMAT_ARGB8888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_BGR;
                    break;
                case SDL_PIXELFORMAT_BGR888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                    break;
                }
                break;
            case SDL_PIXELFORMAT_BGR888:
                switch (renderer->target->format) {
                case SDL_PIXELFORMAT_ABGR8888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_BGR;
                    break;
                case SDL_PIXELFORMAT_ARGB8888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_RGB;
                    break;
                case SDL_PIXELFORMAT_RGB888:
                    sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                    break;
                }
                break;
            case SDL_PIXELFORMAT_IYUV:
            case SDL_PIXELFORMAT_YV12:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_YUV;
                break;
            case SDL_PIXELFORMAT_NV12:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_NV12;
                break;
            case SDL_PIXELFORMAT_NV21:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_NV21;
                break;
            case SDL_PIXELFORMAT_EXTERNAL_OES:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_EXTERNAL_OES;
                break;
            default:
                return SDL_SetError("Unsupported texture format");
            }
        } else {
            sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;   /* Texture formats match, use the non color mapping shader (even if the formats are not ABGR) */
        }
    } else {
        switch (texture->format) {
            case SDL_PIXELFORMAT_ARGB8888:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_ARGB;
                break;
            case SDL_PIXELFORMAT_ABGR8888:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_ABGR;
                break;
            case SDL_PIXELFORMAT_RGB888:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_RGB;
                break;
            case SDL_PIXELFORMAT_BGR888:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_BGR;
                break;
            case SDL_PIXELFORMAT_IYUV:
            case SDL_PIXELFORMAT_YV12:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_YUV;
                break;
            case SDL_PIXELFORMAT_NV12:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_NV12;
                break;
            case SDL_PIXELFORMAT_NV21:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_NV21;
                break;
            case SDL_PIXELFORMAT_EXTERNAL_OES:
                sourceType = GLES2_IMAGESOURCE_TEXTURE_EXTERNAL_OES;
                break;
            default:
                return SDL_SetError("Unsupported texture format");
        }
    }

    if (GLES2_SelectProgram(renderer, sourceType, texture->w, texture->h) < 0) {
        return -1;
    }

    /* Select the target texture */
    if (tdata->yuv) {
        data->glActiveTexture(GL_TEXTURE2);
        data->glBindTexture(tdata->texture_type, tdata->texture_v);

        data->glActiveTexture(GL_TEXTURE1);
        data->glBindTexture(tdata->texture_type, tdata->texture_u);

        data->glActiveTexture(GL_TEXTURE0);
    }
    if (tdata->nv12) {
        data->glActiveTexture(GL_TEXTURE1);
        data->glBindTexture(tdata->texture_type, tdata->texture_u);

        data->glActiveTexture(GL_TEXTURE0);
    }
    data->glBindTexture(tdata->texture_type, tdata->texture);

    /* Configure color modulation */
    g = texture->g;
    a = texture->a;

    if (renderer->target &&
        (renderer->target->format == SDL_PIXELFORMAT_ARGB8888 ||
         renderer->target->format == SDL_PIXELFORMAT_RGB888)) {
        r = texture->b;
        b = texture->r;
    } else {
        r = texture->r;
        b = texture->b;
    }

    program = data->current_program;

    if (!CompareColors(program->modulation_r, program->modulation_g, program->modulation_b, program->modulation_a, r, g, b, a)) {
        data->glUniform4f(program->uniform_locations[GLES2_UNIFORM_MODULATION], r * inv255f, g * inv255f, b * inv255f, a * inv255f);
        program->modulation_r = r;
        program->modulation_g = g;
        program->modulation_b = b;
        program->modulation_a = a;
    }

    /* Configure texture blending */
    GLES2_SetBlendMode(data, texture->blendMode);

    GLES2_SetTexCoords(data, SDL_TRUE);
    return 0;
}

static int
GLES2_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect,
                 const SDL_FRect *dstrect)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLfloat vertices[8];
    GLfloat texCoords[8];

    GLES2_ActivateRenderer(renderer);

    if (GLES2_SetupCopy(renderer, texture) < 0) {
        return -1;
    }

    /* Emit the textured quad */
    vertices[0] = dstrect->x;
    vertices[1] = dstrect->y;
    vertices[2] = (dstrect->x + dstrect->w);
    vertices[3] = dstrect->y;
    vertices[4] = dstrect->x;
    vertices[5] = (dstrect->y + dstrect->h);
    vertices[6] = (dstrect->x + dstrect->w);
    vertices[7] = (dstrect->y + dstrect->h);
    /*data->glVertexAttribPointer(GLES2_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);*/
    GLES2_UpdateVertexBuffer(renderer, GLES2_ATTRIBUTE_POSITION, vertices, 8 * sizeof(GLfloat));
    texCoords[0] = srcrect->x / (GLfloat)texture->w;
    texCoords[1] = srcrect->y / (GLfloat)texture->h;
    texCoords[2] = (srcrect->x + srcrect->w) / (GLfloat)texture->w;
    texCoords[3] = srcrect->y / (GLfloat)texture->h;
    texCoords[4] = srcrect->x / (GLfloat)texture->w;
    texCoords[5] = (srcrect->y + srcrect->h) / (GLfloat)texture->h;
    texCoords[6] = (srcrect->x + srcrect->w) / (GLfloat)texture->w;
    texCoords[7] = (srcrect->y + srcrect->h) / (GLfloat)texture->h;
    /*data->glVertexAttribPointer(GLES2_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, texCoords);*/
    GLES2_UpdateVertexBuffer(renderer, GLES2_ATTRIBUTE_TEXCOORD, texCoords, 8 * sizeof(GLfloat));
    data->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    return GL_CheckError("", renderer);
}

static int
GLES2_RenderCopyEx(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect,
                 const SDL_FRect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLfloat vertices[8];
    GLfloat texCoords[8];
    GLfloat translate[8];
    GLfloat fAngle[4];
    GLfloat tmp;

    GLES2_ActivateRenderer(renderer);

    if (GLES2_SetupCopy(renderer, texture) < 0) {
        return -1;
    }

    data->glEnableVertexAttribArray(GLES2_ATTRIBUTE_CENTER);
    data->glEnableVertexAttribArray(GLES2_ATTRIBUTE_ANGLE);
    fAngle[0] = fAngle[1] = fAngle[2] = fAngle[3] = (GLfloat)(360.0f - angle);
    /* Calculate the center of rotation */
    translate[0] = translate[2] = translate[4] = translate[6] = (center->x + dstrect->x);
    translate[1] = translate[3] = translate[5] = translate[7] = (center->y + dstrect->y);

    /* Emit the textured quad */
    vertices[0] = dstrect->x;
    vertices[1] = dstrect->y;
    vertices[2] = (dstrect->x + dstrect->w);
    vertices[3] = dstrect->y;
    vertices[4] = dstrect->x;
    vertices[5] = (dstrect->y + dstrect->h);
    vertices[6] = (dstrect->x + dstrect->w);
    vertices[7] = (dstrect->y + dstrect->h);
    if (flip & SDL_FLIP_HORIZONTAL) {
        tmp = vertices[0];
        vertices[0] = vertices[4] = vertices[2];
        vertices[2] = vertices[6] = tmp;
    }
    if (flip & SDL_FLIP_VERTICAL) {
        tmp = vertices[1];
        vertices[1] = vertices[3] = vertices[5];
        vertices[5] = vertices[7] = tmp;
    }

    /*data->glVertexAttribPointer(GLES2_ATTRIBUTE_ANGLE, 1, GL_FLOAT, GL_FALSE, 0, &fAngle);
    data->glVertexAttribPointer(GLES2_ATTRIBUTE_CENTER, 2, GL_FLOAT, GL_FALSE, 0, translate);
    data->glVertexAttribPointer(GLES2_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);*/

    GLES2_UpdateVertexBuffer(renderer, GLES2_ATTRIBUTE_ANGLE, fAngle, 4 * sizeof(GLfloat));
    GLES2_UpdateVertexBuffer(renderer, GLES2_ATTRIBUTE_CENTER, translate, 8 * sizeof(GLfloat));
    GLES2_UpdateVertexBuffer(renderer, GLES2_ATTRIBUTE_POSITION, vertices, 8 * sizeof(GLfloat));

    texCoords[0] = srcrect->x / (GLfloat)texture->w;
    texCoords[1] = srcrect->y / (GLfloat)texture->h;
    texCoords[2] = (srcrect->x + srcrect->w) / (GLfloat)texture->w;
    texCoords[3] = srcrect->y / (GLfloat)texture->h;
    texCoords[4] = srcrect->x / (GLfloat)texture->w;
    texCoords[5] = (srcrect->y + srcrect->h) / (GLfloat)texture->h;
    texCoords[6] = (srcrect->x + srcrect->w) / (GLfloat)texture->w;
    texCoords[7] = (srcrect->y + srcrect->h) / (GLfloat)texture->h;
    /*data->glVertexAttribPointer(GLES2_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, texCoords);*/
    GLES2_UpdateVertexBuffer(renderer, GLES2_ATTRIBUTE_TEXCOORD, texCoords, 8 * sizeof(GLfloat));
    data->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    data->glDisableVertexAttribArray(GLES2_ATTRIBUTE_CENTER);
    data->glDisableVertexAttribArray(GLES2_ATTRIBUTE_ANGLE);

    return GL_CheckError("", renderer);
}

static int
GLES2_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                    Uint32 pixel_format, void * pixels, int pitch)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    Uint32 temp_format = renderer->target ? renderer->target->format : SDL_PIXELFORMAT_ABGR8888;
    void *temp_pixels;
    int temp_pitch;
    Uint8 *src, *dst, *tmp;
    int w, h, length, rows;
    int status;

    GLES2_ActivateRenderer(renderer);

    temp_pitch = rect->w * SDL_BYTESPERPIXEL(temp_format);
    temp_pixels = SDL_malloc(rect->h * temp_pitch);
    if (!temp_pixels) {
        return SDL_OutOfMemory();
    }

    SDL_GetRendererOutputSize(renderer, &w, &h);

    data->glReadPixels(rect->x, renderer->target ? rect->y : (h-rect->y)-rect->h,
                       rect->w, rect->h, GL_RGBA, GL_UNSIGNED_BYTE, temp_pixels);
    if (GL_CheckError("glReadPixels()", renderer) < 0) {
        return -1;
    }

    /* Flip the rows to be top-down if necessary */
    if (!renderer->target) {
        length = rect->w * SDL_BYTESPERPIXEL(temp_format);
        src = (Uint8*)temp_pixels + (rect->h-1)*temp_pitch;
        dst = (Uint8*)temp_pixels;
        tmp = SDL_stack_alloc(Uint8, length);
        rows = rect->h / 2;
        while (rows--) {
            SDL_memcpy(tmp, dst, length);
            SDL_memcpy(dst, src, length);
            SDL_memcpy(src, tmp, length);
            dst += temp_pitch;
            src -= temp_pitch;
        }
        SDL_stack_free(tmp);
    }

    status = SDL_ConvertPixels(rect->w, rect->h,
                               temp_format, temp_pixels, temp_pitch,
                               pixel_format, pixels, pitch);
    SDL_free(temp_pixels);

    return status;
}

static void
GLES2_RenderPresent(SDL_Renderer *renderer)
{
    GLES2_ActivateRenderer(renderer);

    /* Tell the video driver to swap buffers */
    SDL_GL_SwapWindow(renderer->window);
}


/*************************************************************************************************
 * Bind/unbinding of textures
 *************************************************************************************************/
static int GLES2_BindTexture (SDL_Renderer * renderer, SDL_Texture *texture, float *texw, float *texh);
static int GLES2_UnbindTexture (SDL_Renderer * renderer, SDL_Texture *texture);

static int GLES2_BindTexture (SDL_Renderer * renderer, SDL_Texture *texture, float *texw, float *texh)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_TextureData *texturedata = (GLES2_TextureData *)texture->driverdata;
    GLES2_ActivateRenderer(renderer);

    data->glBindTexture(texturedata->texture_type, texturedata->texture);

    if (texw) {
        *texw = 1.0;
    }
    if (texh) {
        *texh = 1.0;
    }

    return 0;
}

static int GLES2_UnbindTexture (SDL_Renderer * renderer, SDL_Texture *texture)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *)renderer->driverdata;
    GLES2_TextureData *texturedata = (GLES2_TextureData *)texture->driverdata;
    GLES2_ActivateRenderer(renderer);

    data->glBindTexture(texturedata->texture_type, 0);

    return 0;
}


/*************************************************************************************************
 * Renderer instantiation                                                                        *
 *************************************************************************************************/

#ifdef ZUNE_HD
#define GL_NVIDIA_PLATFORM_BINARY_NV 0x890B
#endif

static void
GLES2_ResetState(SDL_Renderer *renderer)
{
    GLES2_DriverContext *data = (GLES2_DriverContext *) renderer->driverdata;

    if (SDL_CurrentContext == data->context) {
        GLES2_UpdateViewport(renderer);
    } else {
        GLES2_ActivateRenderer(renderer);
    }

    data->current.blendMode = SDL_BLENDMODE_INVALID;
    data->current.tex_coords = SDL_FALSE;

    data->glActiveTexture(GL_TEXTURE0);
    data->glPixelStorei(GL_PACK_ALIGNMENT, 1);
    data->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    data->glClearColor((GLfloat) data->clear_r * inv255f,
                        (GLfloat) data->clear_g * inv255f,
                        (GLfloat) data->clear_b * inv255f,
                        (GLfloat) data->clear_a * inv255f);

    data->glEnableVertexAttribArray(GLES2_ATTRIBUTE_POSITION);
    data->glDisableVertexAttribArray(GLES2_ATTRIBUTE_TEXCOORD);

    GL_CheckError("", renderer);
}

static SDL_Renderer *
GLES2_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer;
    GLES2_DriverContext *data;
    GLint nFormats;
#ifndef ZUNE_HD
    GLboolean hasCompiler;
#endif
    Uint32 window_flags = 0; /* -Wconditional-uninitialized */
    GLint window_framebuffer;
    GLint value;
    int profile_mask = 0, major = 0, minor = 0;
    SDL_bool changed_window = SDL_FALSE;

    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile_mask) < 0) {
        goto error;
    }
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major) < 0) {
        goto error;
    }
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor) < 0) {
        goto error;
    }

    window_flags = SDL_GetWindowFlags(window);
    /* OpenGL ES 3.0 is a superset of OpenGL ES 2.0 */
    if (!(window_flags & SDL_WINDOW_OPENGL) ||
        profile_mask != SDL_GL_CONTEXT_PROFILE_ES || major < RENDERER_CONTEXT_MAJOR) {

        changed_window = SDL_TRUE;
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, RENDERER_CONTEXT_MAJOR);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, RENDERER_CONTEXT_MINOR);

        if (SDL_RecreateWindow(window, window_flags | SDL_WINDOW_OPENGL) < 0) {
            goto error;
        }
    }

    /* Create the renderer struct */
    renderer = (SDL_Renderer *)SDL_calloc(1, sizeof(SDL_Renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        goto error;
    }

    data = (GLES2_DriverContext *)SDL_calloc(1, sizeof(GLES2_DriverContext));
    if (!data) {
        GLES2_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        goto error;
    }
    renderer->info = GLES2_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;
    renderer->window = window;

    /* Create an OpenGL ES 2.0 context */
    data->context = SDL_GL_CreateContext(window);
    if (!data->context) {
        GLES2_DestroyRenderer(renderer);
        goto error;
    }
    if (SDL_GL_MakeCurrent(window, data->context) < 0) {
        GLES2_DestroyRenderer(renderer);
        goto error;
    }

    if (GLES2_LoadFunctions(data) < 0) {
        GLES2_DestroyRenderer(renderer);
        goto error;
    }

#if __WINRT__
    /* DLudwig, 2013-11-29: ANGLE for WinRT doesn't seem to work unless VSync
     * is turned on.  Not doing so will freeze the screen's contents to that
     * of the first drawn frame.
     */
    flags |= SDL_RENDERER_PRESENTVSYNC;
#endif

    if (flags & SDL_RENDERER_PRESENTVSYNC) {
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_GL_SetSwapInterval(0);
    }
    if (SDL_GL_GetSwapInterval() > 0) {
        renderer->info.flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    /* Check for debug output support */
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &value) == 0 &&
        (value & SDL_GL_CONTEXT_DEBUG_FLAG)) {
        data->debug_enabled = SDL_TRUE;
    }

    value = 0;
    data->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    renderer->info.max_texture_width = value;
    value = 0;
    data->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    renderer->info.max_texture_height = value;

    /* Determine supported shader formats */
    /* HACK: glGetInteger is broken on the Zune HD's compositor, so we just hardcode this */
#ifdef ZUNE_HD
    nFormats = 1;
#else /* !ZUNE_HD */
    data->glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &nFormats);
    data->glGetBooleanv(GL_SHADER_COMPILER, &hasCompiler);
    if (hasCompiler) {
        ++nFormats;
    }
#endif /* ZUNE_HD */
    data->shader_formats = (GLenum *)SDL_calloc(nFormats, sizeof(GLenum));
    if (!data->shader_formats) {
        GLES2_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        goto error;
    }
    data->shader_format_count = nFormats;
#ifdef ZUNE_HD
    data->shader_formats[0] = GL_NVIDIA_PLATFORM_BINARY_NV;
#else /* !ZUNE_HD */
    data->glGetIntegerv(GL_SHADER_BINARY_FORMATS, (GLint *)data->shader_formats);
    if (hasCompiler) {
        data->shader_formats[nFormats - 1] = (GLenum)-1;
    }
#endif /* ZUNE_HD */

    data->framebuffers = NULL;
    data->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &window_framebuffer);
    data->window_framebuffer = (GLuint)window_framebuffer;

    /* Populate the function pointers for the module */
    renderer->WindowEvent         = GLES2_WindowEvent;
    renderer->GetOutputSize       = GLES2_GetOutputSize;
    renderer->SupportsBlendMode   = GLES2_SupportsBlendMode;
    renderer->CreateTexture       = GLES2_CreateTexture;
    renderer->UpdateTexture       = GLES2_UpdateTexture;
    renderer->UpdateTextureYUV    = GLES2_UpdateTextureYUV;
    renderer->LockTexture         = GLES2_LockTexture;
    renderer->UnlockTexture       = GLES2_UnlockTexture;
    renderer->SetRenderTarget     = GLES2_SetRenderTarget;
    renderer->UpdateViewport      = GLES2_UpdateViewport;
    renderer->UpdateClipRect      = GLES2_UpdateClipRect;
    renderer->RenderClear         = GLES2_RenderClear;
    renderer->RenderDrawPoints    = GLES2_RenderDrawPoints;
    renderer->RenderDrawLines     = GLES2_RenderDrawLines;
    renderer->RenderFillRects     = GLES2_RenderFillRects;
    renderer->RenderCopy          = GLES2_RenderCopy;
    renderer->RenderCopyEx        = GLES2_RenderCopyEx;
    renderer->RenderReadPixels    = GLES2_RenderReadPixels;
    renderer->RenderPresent       = GLES2_RenderPresent;
    renderer->DestroyTexture      = GLES2_DestroyTexture;
    renderer->DestroyRenderer     = GLES2_DestroyRenderer;
    renderer->GL_BindTexture      = GLES2_BindTexture;
    renderer->GL_UnbindTexture    = GLES2_UnbindTexture;

    renderer->info.texture_formats[renderer->info.num_texture_formats++] = SDL_PIXELFORMAT_YV12;
    renderer->info.texture_formats[renderer->info.num_texture_formats++] = SDL_PIXELFORMAT_IYUV;
    renderer->info.texture_formats[renderer->info.num_texture_formats++] = SDL_PIXELFORMAT_NV12;
    renderer->info.texture_formats[renderer->info.num_texture_formats++] = SDL_PIXELFORMAT_NV21;
#ifdef GL_TEXTURE_EXTERNAL_OES
    renderer->info.texture_formats[renderer->info.num_texture_formats++] = SDL_PIXELFORMAT_EXTERNAL_OES;
#endif

    GLES2_ResetState(renderer);

    return renderer;

error:
    if (changed_window) {
        /* Uh oh, better try to put it back... */
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile_mask);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
        SDL_RecreateWindow(window, window_flags);
    }
    return NULL;
}

#endif /* SDL_VIDEO_RENDER_OGL_ES2 && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
