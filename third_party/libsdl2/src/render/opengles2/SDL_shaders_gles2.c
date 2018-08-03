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

#include "SDL_video.h"
#include "SDL_opengles2.h"
#include "SDL_shaders_gles2.h"
#include "SDL_stdinc.h"

/*************************************************************************************************
 * Vertex/fragment shader source                                                                 *
 *************************************************************************************************/

static const Uint8 GLES2_VertexSrc_Default_[] = " \
    uniform mat4 u_projection; \
    attribute vec2 a_position; \
    attribute vec2 a_texCoord; \
    attribute float a_angle; \
    attribute vec2 a_center; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        float angle = radians(a_angle); \
        float c = cos(angle); \
        float s = sin(angle); \
        mat2 rotationMatrix = mat2(c, -s, s, c); \
        vec2 position = rotationMatrix * (a_position - a_center) + a_center; \
        v_texCoord = a_texCoord; \
        gl_Position = u_projection * vec4(position, 0.0, 1.0);\
        gl_PointSize = 1.0; \
    } \
";

static const Uint8 GLES2_FragmentSrc_SolidSrc_[] = " \
    precision mediump float; \
    uniform vec4 u_color; \
    \
    void main() \
    { \
        gl_FragColor = u_color; \
    } \
";

static const Uint8 GLES2_FragmentSrc_TextureABGRSrc_[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_modulation; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        gl_FragColor = texture2D(u_texture, v_texCoord); \
        gl_FragColor *= u_modulation; \
    } \
";

/* ARGB to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureARGBSrc_[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_modulation; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
        gl_FragColor = abgr; \
        gl_FragColor.r = abgr.b; \
        gl_FragColor.b = abgr.r; \
        gl_FragColor *= u_modulation; \
    } \
";

/* RGB to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureRGBSrc_[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_modulation; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
        gl_FragColor = abgr; \
        gl_FragColor.r = abgr.b; \
        gl_FragColor.b = abgr.r; \
        gl_FragColor.a = 1.0; \
        gl_FragColor *= u_modulation; \
    } \
";

/* BGR to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureBGRSrc_[] = " \
    precision mediump float; \
    uniform sampler2D u_texture; \
    uniform vec4 u_modulation; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        vec4 abgr = texture2D(u_texture, v_texCoord); \
        gl_FragColor = abgr; \
        gl_FragColor.a = 1.0; \
        gl_FragColor *= u_modulation; \
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
"uniform vec4 u_modulation;\n"                                  \
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
"    gl_FragColor *= u_modulation;\n"                           \
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
"    gl_FragColor *= u_modulation;\n"                           \
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
"    gl_FragColor *= u_modulation;\n"                           \
"}"                                                             \

/* YUV to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureYUVJPEGSrc_[] = \
        YUV_SHADER_PROLOGUE \
        JPEG_SHADER_CONSTANTS \
        YUV_SHADER_BODY \
;
static const Uint8 GLES2_FragmentSrc_TextureYUVBT601Src_[] = \
        YUV_SHADER_PROLOGUE \
        BT601_SHADER_CONSTANTS \
        YUV_SHADER_BODY \
;
static const Uint8 GLES2_FragmentSrc_TextureYUVBT709Src_[] = \
        YUV_SHADER_PROLOGUE \
        BT709_SHADER_CONSTANTS \
        YUV_SHADER_BODY \
;

/* NV12 to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureNV12JPEGSrc_[] = \
        YUV_SHADER_PROLOGUE \
        JPEG_SHADER_CONSTANTS \
        NV12_SHADER_BODY \
;
static const Uint8 GLES2_FragmentSrc_TextureNV12BT601Src_[] = \
        YUV_SHADER_PROLOGUE \
        BT601_SHADER_CONSTANTS \
        NV12_SHADER_BODY \
;
static const Uint8 GLES2_FragmentSrc_TextureNV12BT709Src_[] = \
        YUV_SHADER_PROLOGUE \
        BT709_SHADER_CONSTANTS \
        NV12_SHADER_BODY \
;

/* NV21 to ABGR conversion */
static const Uint8 GLES2_FragmentSrc_TextureNV21JPEGSrc_[] = \
        YUV_SHADER_PROLOGUE \
        JPEG_SHADER_CONSTANTS \
        NV21_SHADER_BODY \
;
static const Uint8 GLES2_FragmentSrc_TextureNV21BT601Src_[] = \
        YUV_SHADER_PROLOGUE \
        BT601_SHADER_CONSTANTS \
        NV21_SHADER_BODY \
;
static const Uint8 GLES2_FragmentSrc_TextureNV21BT709Src_[] = \
        YUV_SHADER_PROLOGUE \
        BT709_SHADER_CONSTANTS \
        NV21_SHADER_BODY \
;

/* Custom Android video format texture */
static const Uint8 GLES2_FragmentSrc_TextureExternalOESSrc_[] = " \
    #extension GL_OES_EGL_image_external : require\n\
    precision mediump float; \
    uniform samplerExternalOES u_texture; \
    uniform vec4 u_modulation; \
    varying vec2 v_texCoord; \
    \
    void main() \
    { \
        gl_FragColor = texture2D(u_texture, v_texCoord); \
        gl_FragColor *= u_modulation; \
    } \
";

static const GLES2_ShaderInstance GLES2_VertexSrc_Default = {
    GL_VERTEX_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_VertexSrc_Default_),
    GLES2_VertexSrc_Default_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_SolidSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_SolidSrc_),
    GLES2_FragmentSrc_SolidSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureABGRSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureABGRSrc_),
    GLES2_FragmentSrc_TextureABGRSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureARGBSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureARGBSrc_),
    GLES2_FragmentSrc_TextureARGBSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureRGBSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureRGBSrc_),
    GLES2_FragmentSrc_TextureRGBSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureBGRSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureBGRSrc_),
    GLES2_FragmentSrc_TextureBGRSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureYUVJPEGSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureYUVJPEGSrc_),
    GLES2_FragmentSrc_TextureYUVJPEGSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureYUVBT601Src = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureYUVBT601Src_),
    GLES2_FragmentSrc_TextureYUVBT601Src_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureYUVBT709Src = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureYUVBT709Src_),
    GLES2_FragmentSrc_TextureYUVBT709Src_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureNV12JPEGSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureNV12JPEGSrc_),
    GLES2_FragmentSrc_TextureNV12JPEGSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureNV12BT601Src = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureNV12BT601Src_),
    GLES2_FragmentSrc_TextureNV12BT601Src_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureNV21BT709Src = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureNV21BT709Src_),
    GLES2_FragmentSrc_TextureNV21BT709Src_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureNV21JPEGSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureNV21JPEGSrc_),
    GLES2_FragmentSrc_TextureNV21JPEGSrc_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureNV21BT601Src = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureNV21BT601Src_),
    GLES2_FragmentSrc_TextureNV21BT601Src_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureNV12BT709Src = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureNV12BT709Src_),
    GLES2_FragmentSrc_TextureNV12BT709Src_
};

static const GLES2_ShaderInstance GLES2_FragmentSrc_TextureExternalOESSrc = {
    GL_FRAGMENT_SHADER,
    GLES2_SOURCE_SHADER,
    sizeof(GLES2_FragmentSrc_TextureExternalOESSrc_),
    GLES2_FragmentSrc_TextureExternalOESSrc_
};


/*************************************************************************************************
 * Vertex/fragment shader definitions                                                            *
 *************************************************************************************************/

static GLES2_Shader GLES2_VertexShader_Default = {
    1,
    {
        &GLES2_VertexSrc_Default
    }
};

static GLES2_Shader GLES2_FragmentShader_SolidSrc = {
    1,
    {
        &GLES2_FragmentSrc_SolidSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureABGRSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureABGRSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureARGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureARGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureRGBSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureRGBSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureBGRSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureBGRSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureYUVJPEGSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureYUVJPEGSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureYUVBT601Src = {
    1,
    {
        &GLES2_FragmentSrc_TextureYUVBT601Src
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureYUVBT709Src = {
    1,
    {
        &GLES2_FragmentSrc_TextureYUVBT709Src
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureNV12JPEGSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureNV12JPEGSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureNV12BT601Src = {
    1,
    {
        &GLES2_FragmentSrc_TextureNV12BT601Src
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureNV12BT709Src = {
    1,
    {
        &GLES2_FragmentSrc_TextureNV12BT709Src
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureNV21JPEGSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureNV21JPEGSrc
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureNV21BT601Src = {
    1,
    {
        &GLES2_FragmentSrc_TextureNV21BT601Src
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureNV21BT709Src = {
    1,
    {
        &GLES2_FragmentSrc_TextureNV21BT709Src
    }
};

static GLES2_Shader GLES2_FragmentShader_TextureExternalOESSrc = {
    1,
    {
        &GLES2_FragmentSrc_TextureExternalOESSrc
    }
};


/*************************************************************************************************
 * Shader selector                                                                               *
 *************************************************************************************************/

const GLES2_Shader *GLES2_GetShader(GLES2_ShaderType type)
{
    switch (type) {
    case GLES2_SHADER_VERTEX_DEFAULT:
        return &GLES2_VertexShader_Default;
    case GLES2_SHADER_FRAGMENT_SOLID_SRC:
        return &GLES2_FragmentShader_SolidSrc;
    case GLES2_SHADER_FRAGMENT_TEXTURE_ABGR_SRC:
        return &GLES2_FragmentShader_TextureABGRSrc;
    case GLES2_SHADER_FRAGMENT_TEXTURE_ARGB_SRC:
        return &GLES2_FragmentShader_TextureARGBSrc;
    case GLES2_SHADER_FRAGMENT_TEXTURE_RGB_SRC:
        return &GLES2_FragmentShader_TextureRGBSrc;
    case GLES2_SHADER_FRAGMENT_TEXTURE_BGR_SRC:
        return &GLES2_FragmentShader_TextureBGRSrc;
    case GLES2_SHADER_FRAGMENT_TEXTURE_YUV_JPEG_SRC:
        return &GLES2_FragmentShader_TextureYUVJPEGSrc;
    case GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT601_SRC:
        return &GLES2_FragmentShader_TextureYUVBT601Src;
    case GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT709_SRC:
        return &GLES2_FragmentShader_TextureYUVBT709Src;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV12_JPEG_SRC:
        return &GLES2_FragmentShader_TextureNV12JPEGSrc;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT601_SRC:
        return &GLES2_FragmentShader_TextureNV12BT601Src;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT709_SRC:
        return &GLES2_FragmentShader_TextureNV12BT709Src;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV21_JPEG_SRC:
        return &GLES2_FragmentShader_TextureNV21JPEGSrc;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT601_SRC:
        return &GLES2_FragmentShader_TextureNV21BT601Src;
    case GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT709_SRC:
        return &GLES2_FragmentShader_TextureNV21BT709Src;
    case GLES2_SHADER_FRAGMENT_TEXTURE_EXTERNAL_OES_SRC:
        return &GLES2_FragmentShader_TextureExternalOESSrc;
    default:
        return NULL;
    }
}

#endif /* SDL_VIDEO_RENDER_OGL_ES2 && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
