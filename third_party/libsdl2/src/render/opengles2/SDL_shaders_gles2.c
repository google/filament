/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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

#include "SDL_video.h"
#include "SDL_opengles2.h"
#include "SDL_shaders_gles2.h"
#include "SDL_stdinc.h"

/*************************************************************************************************
 * Vertex/fragment shader source                                                                 *
 *************************************************************************************************/
/* Notes on a_angle:
   * It is a vector containing sin and cos for rotation matrix
   * To get correct rotation for most cases when a_angle is disabled cos
     value is decremented by 1.0 to get proper output with 0.0 which is
     default value
*/
static const Uint8 GLES2_Vertex_Default[] = " \
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

static const Uint8 GLES2_Fragment_Solid[] = " \
    precision mediump float; \
    uniform vec4 u_color; \
    \
    void main() \
    { \
        gl_FragColor = u_color; \
    } \
";

static const Uint8 GLES2_Fragment_TextureABGR[] = " \
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

/* ARGB to ABGR conversion */
static const Uint8 GLES2_Fragment_TextureARGB[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_color; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
        gl_FragColor = abgr; \
        gl_FragColor.r = abgr.b; \
        gl_FragColor.b = abgr.r; \
        gl_FragColor *= u_color; \
    } \
";

/* RGB to ABGR conversion */
static const Uint8 GLES2_Fragment_TextureRGB[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_color; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
        gl_FragColor = abgr; \
        gl_FragColor.r = abgr.b; \
        gl_FragColor.b = abgr.r; \
        gl_FragColor.a = 1.0; \
        gl_FragColor *= u_color; \
    } \
";

/* BGR to ABGR conversion */
static const Uint8 GLES2_Fragment_TextureBGR[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_color; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
        gl_FragColor = abgr; \
        gl_FragColor.a = 1.0; \
        gl_FragColor *= u_color; \
    } \
";

#define JPEG_SHADER_CONSTANTS                                   \
"// YUV offset \n"                                              \
"const vec3 offset = vec3(0, -0.501960814, -0.501960814);\n"    \
"\n"                                                            \
"// RGB coefficients \n"                                        \
"const mat3 matrix = mat3( 1,       1,        1,\n"             \
"                          0,      -0.3441,   1.772,\n"         \
"                          1.402,  -0.7141,   0);\n"            \

#define BT601_SHADER_CONSTANTS                                  \
"// YUV offset \n"                                              \
"const vec3 offset = vec3(-0.0627451017, -0.501960814, -0.501960814);\n" \
"\n"                                                            \
"// RGB coefficients \n"                                        \
"const mat3 matrix = mat3( 1.1644,  1.1644,   1.1644,\n"        \
"                          0,      -0.3918,   2.0172,\n"        \
"                          1.596,  -0.813,    0);\n"            \

#define BT709_SHADER_CONSTANTS                                  \
"// YUV offset \n"                                              \
"const vec3 offset = vec3(-0.0627451017, -0.501960814, -0.501960814);\n" \
"\n"                                                            \
"// RGB coefficients \n"                                        \
"const mat3 matrix = mat3( 1.1644,  1.1644,   1.1644,\n"        \
"                          0,      -0.2132,   2.1124,\n"        \
"                          1.7927, -0.5329,   0);\n"            \


#define YUV_SHADER_PROLOGUE                                     \
"precision mediump float;\n"                                    \
"uniform sampler2D u_texture;\n"                                \
"uniform sampler2D u_texture_u;\n"                              \
"uniform sampler2D u_texture_v;\n"                              \
"uniform vec4 u_color;\n"                                  \
"varying vec2 v_texCoord;\n"                                    \
"\n"                                                            \

#define YUV_SHADER_BODY                                         \
"\n"                                                            \
"void main()\n"                                                 \
"{\n"                                                           \
"    mediump vec3 yuv;\n"                                       \
"    lowp vec3 rgb;\n"                                          \
"\n"                                                            \
"    // Get the YUV values \n"                                  \
"    yuv.x = texture2D(u_texture,   v_texCoord).r;\n"           \
"    yuv.y = texture2D(u_texture_u, v_texCoord).r;\n"           \
"    yuv.z = texture2D(u_texture_v, v_texCoord).r;\n"           \
"\n"                                                            \
"    // Do the color transform \n"                              \
"    yuv += offset;\n"                                          \
"    rgb = matrix * yuv;\n"                                     \
"\n"                                                            \
"    // That was easy. :) \n"                                   \
"    gl_FragColor = vec4(rgb, 1);\n"                            \
"    gl_FragColor *= u_color;\n"                           \
"}"                                                             \

#define NV12_SHADER_BODY                                        \
"\n"                                                            \
"void main()\n"                                                 \
"{\n"                                                           \
"    mediump vec3 yuv;\n"                                       \
"    lowp vec3 rgb;\n"                                          \
"\n"                                                            \
"    // Get the YUV values \n"                                  \
"    yuv.x = texture2D(u_texture,   v_texCoord).r;\n"           \
"    yuv.yz = texture2D(u_texture_u, v_texCoord).ra;\n"         \
"\n"                                                            \
"    // Do the color transform \n"                              \
"    yuv += offset;\n"                                          \
"    rgb = matrix * yuv;\n"                                     \
"\n"                                                            \
"    // That was easy. :) \n"                                   \
"    gl_FragColor = vec4(rgb, 1);\n"                            \
"    gl_FragColor *= u_color;\n"                           \
"}"                                                             \

#define NV21_SHADER_BODY                                        \
"\n"                                                            \
"void main()\n"                                                 \
"{\n"                                                           \
"    mediump vec3 yuv;\n"                                       \
"    lowp vec3 rgb;\n"                                          \
"\n"                                                            \
"    // Get the YUV values \n"                                  \
"    yuv.x = texture2D(u_texture,   v_texCoord).r;\n"           \
"    yuv.yz = texture2D(u_texture_u, v_texCoord).ar;\n"         \
"\n"                                                            \
"    // Do the color transform \n"                              \
"    yuv += offset;\n"                                          \
"    rgb = matrix * yuv;\n"                                     \
"\n"                                                            \
"    // That was easy. :) \n"                                   \
"    gl_FragColor = vec4(rgb, 1);\n"                            \
"    gl_FragColor *= u_color;\n"                           \
"}"                                                             \

/* YUV to ABGR conversion */
static const Uint8 GLES2_Fragment_TextureYUVJPEG[] = \
        YUV_SHADER_PROLOGUE \
        JPEG_SHADER_CONSTANTS \
        YUV_SHADER_BODY \
;
static const Uint8 GLES2_Fragment_TextureYUVBT601[] = \
        YUV_SHADER_PROLOGUE \
        BT601_SHADER_CONSTANTS \
        YUV_SHADER_BODY \
;
static const Uint8 GLES2_Fragment_TextureYUVBT709[] = \
        YUV_SHADER_PROLOGUE \
        BT709_SHADER_CONSTANTS \
        YUV_SHADER_BODY \
;

/* NV12 to ABGR conversion */
static const Uint8 GLES2_Fragment_TextureNV12JPEG[] = \
        YUV_SHADER_PROLOGUE \
        JPEG_SHADER_CONSTANTS \
        NV12_SHADER_BODY \
;
static const Uint8 GLES2_Fragment_TextureNV12BT601[] = \
        YUV_SHADER_PROLOGUE \
        BT601_SHADER_CONSTANTS \
        NV12_SHADER_BODY \
;
static const Uint8 GLES2_Fragment_TextureNV12BT709[] = \
        YUV_SHADER_PROLOGUE \
        BT709_SHADER_CONSTANTS \
        NV12_SHADER_BODY \
;

/* NV21 to ABGR conversion */
static const Uint8 GLES2_Fragment_TextureNV21JPEG[] = \
        YUV_SHADER_PROLOGUE \
        JPEG_SHADER_CONSTANTS \
        NV21_SHADER_BODY \
;
static const Uint8 GLES2_Fragment_TextureNV21BT601[] = \
        YUV_SHADER_PROLOGUE \
        BT601_SHADER_CONSTANTS \
        NV21_SHADER_BODY \
;
static const Uint8 GLES2_Fragment_TextureNV21BT709[] = \
        YUV_SHADER_PROLOGUE \
        BT709_SHADER_CONSTANTS \
        NV21_SHADER_BODY \
;

/* Custom Android video format texture */
static const Uint8 GLES2_Fragment_TextureExternalOES[] = " \
    #extension GL_OES_EGL_image_external : require\n\
    precision mediump float; \
    uniform samplerExternalOES u_texture; \
    uniform vec4 u_color; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        gl_FragColor = texture2D(u_texture, v_texCoord); \
        gl_FragColor *= u_color; \
    } \
";


/*************************************************************************************************
 * Shader selector                                                                               *
 *************************************************************************************************/

const Uint8 *GLES2_GetShader(GLES2_ShaderType type)
{
    switch (type) {
    case GLES2_SHADER_VERTEX_DEFAULT:
        return GLES2_Vertex_Default;
    case GLES2_SHADER_FRAGMENT_SOLID:
        return GLES2_Fragment_Solid;
    case GLES2_SHADER_FRAGMENT_TEXTURE_ABGR:
        return GLES2_Fragment_TextureABGR;
    case GLES2_SHADER_FRAGMENT_TEXTURE_ARGB:
        return GLES2_Fragment_TextureARGB;
    case GLES2_SHADER_FRAGMENT_TEXTURE_RGB:
        return GLES2_Fragment_TextureRGB;
    case GLES2_SHADER_FRAGMENT_TEXTURE_BGR:
        return GLES2_Fragment_TextureBGR;
    case GLES2_SHADER_FRAGMENT_TEXTURE_YUV_JPEG:
        return GLES2_Fragment_TextureYUVJPEG;
    case GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT601:
        return GLES2_Fragment_TextureYUVBT601;
    case GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT709:
        return GLES2_Fragment_TextureYUVBT709;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV12_JPEG:
        return GLES2_Fragment_TextureNV12JPEG;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT601:
        return GLES2_Fragment_TextureNV12BT601;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT709:
        return GLES2_Fragment_TextureNV12BT709;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV21_JPEG:
        return GLES2_Fragment_TextureNV21JPEG;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT601:
        return GLES2_Fragment_TextureNV21BT601;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT709:
        return GLES2_Fragment_TextureNV21BT709;
    case GLES2_SHADER_FRAGMENT_TEXTURE_EXTERNAL_OES:
        return GLES2_Fragment_TextureExternalOES;
    default:
        return NULL;
    }
}

#endif /* SDL_VIDEO_RENDER_OGL_ES2 && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
