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

/* D3D11 shader implementation */

typedef enum {
    SHADER_SOLID,
    SHADER_RGB,
    SHADER_YUV_JPEG,
    SHADER_YUV_BT601,
    SHADER_YUV_BT709,
    SHADER_NV12_JPEG,
    SHADER_NV12_BT601,
    SHADER_NV12_BT709,
    SHADER_NV21_JPEG,
    SHADER_NV21_BT601,
    SHADER_NV21_BT709,
    NUM_SHADERS
} D3D11_Shader;

extern int D3D11_CreateVertexShader(ID3D11Device1 *d3dDevice, ID3D11VertexShader **vertexShader, ID3D11InputLayout **inputLayout);
extern int D3D11_CreatePixelShader(ID3D11Device1 *d3dDevice, D3D11_Shader shader, ID3D11PixelShader **pixelShader);

/* vi: set ts=4 sw=4 expandtab: */
