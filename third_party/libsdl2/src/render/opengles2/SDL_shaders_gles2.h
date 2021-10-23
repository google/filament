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

#ifndef SDL_shaders_gles2_h_
#define SDL_shaders_gles2_h_

#if SDL_VIDEO_RENDER_OGL_ES2


typedef enum
{
    GLES2_SHADER_VERTEX_DEFAULT = 0,
    GLES2_SHADER_FRAGMENT_SOLID,
    GLES2_SHADER_FRAGMENT_TEXTURE_ABGR,
    GLES2_SHADER_FRAGMENT_TEXTURE_ARGB,
    GLES2_SHADER_FRAGMENT_TEXTURE_BGR,
    GLES2_SHADER_FRAGMENT_TEXTURE_RGB,
    GLES2_SHADER_FRAGMENT_TEXTURE_YUV_JPEG,
    GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT601,
    GLES2_SHADER_FRAGMENT_TEXTURE_YUV_BT709,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV12_JPEG,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT601,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV12_BT709,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV21_JPEG,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT601,
    GLES2_SHADER_FRAGMENT_TEXTURE_NV21_BT709,
    GLES2_SHADER_FRAGMENT_TEXTURE_EXTERNAL_OES
} GLES2_ShaderType;

#define GLES2_SHADER_COUNT  16

const Uint8 *GLES2_GetShader(GLES2_ShaderType type);

#endif /* SDL_VIDEO_RENDER_OGL_ES2 */

#endif /* SDL_shaders_gles2_h_ */

/* vi: set ts=4 sw=4 expandtab: */
