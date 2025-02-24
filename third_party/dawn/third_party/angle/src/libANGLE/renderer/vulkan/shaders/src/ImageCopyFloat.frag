//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageCopyFloat.frag: Copy parts of a YUV or multisampled image to another.

#version 450 core

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_samplerless_texture_functions : require

#define SrcIsFloat 1
#define DstIsFloat 1
#define SrcType vec4
#define DstType vec4

#if SrcIsYUV
#define SRC_RESOURCE_NAME sampler2D
#elif SrcIs2DMS
#define SRC_RESOURCE_NAME texture2DMS
#else
#error "Not all source types are accounted for"
#endif

layout(set = 0, binding = 0) uniform SRC_RESOURCE_NAME src;
layout(location = 0) out DstType dst;

#include "ImageCopy.inc"

void main()
{
    ivec2 srcSubImageCoords = transformImageCoords(ivec2(gl_FragCoord.xy));

#if SrcIsYUV
    SrcType srcValue = texture(
        src, vec2(params.srcOffset + srcSubImageCoords) / textureSize(src, 0), params.srcMip);
#elif SrcIs2DMS
    SrcType srcValue = SrcType(0);
    for (int i = 0; i < params.srcSampleCount; i++)
    {
        srcValue += texelFetch(src, ivec2(params.srcOffset + srcSubImageCoords), i);
    }
    srcValue /= params.srcSampleCount;
#else
#error "Not all source types are accounted for"
#endif

    dst = transformSrcValue(srcValue);
}
