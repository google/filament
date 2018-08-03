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

#ifndef SDL_shaders_gles2_h_
#define SDL_shaders_gles2_h_

#if SDL_VIDEO_RENDER_OGL_ES2

typedef struct GLES2_ShaderInstance
{
    GLenum type;
    GLenum format;
    int length;
    const void *data;
} GLES2_ShaderInstance;

typedef struct GLES2_Shader
{
    int instance_count;
    const GLES2_ShaderInstance *instances[4];
} GLES2_Shader;

typedef enum
{
    GLES2_SHADER_VERTEX_DEFAULT,
    GLES2_SHADER_FRAGMENT_SOLID_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_ABGR_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_ARGB_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_BGR_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_RGB_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_YUV_JPEG_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT601_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT709_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV12_JPEG_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT601_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT709_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV21_JPEG_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT601_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT709_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_EXTERNAL_OES_SRC
} GLES2_ShaderType;

#define GLES2_SOURCE_SHADER (GLenum)-1

const GLES2_Shader *GLES2_GetShader(GLES2_ShaderType type);

#endif /* SDL_VIDEO_RENDER_OGL_ES2 */

#endif /* SDL_shaders_gles2_h_ */

/* vi: set ts=4 sw=4 expandtab: */
