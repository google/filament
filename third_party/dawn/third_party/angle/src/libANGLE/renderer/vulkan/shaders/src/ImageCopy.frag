//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageCopy.frag: Copy parts of an image to another.

#version 450 core

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_samplerless_texture_functions : require

#define MAKE_SRC_RESOURCE(prefix, type) prefix ## type

#if SrcIsFloat
#define SRC_RESOURCE(type) type
#define SrcType vec4
#elif SrcIsSint
#define SRC_RESOURCE(type) MAKE_SRC_RESOURCE(i, type)
#define SrcType ivec4
#elif SrcIsUint
#define SRC_RESOURCE(type) MAKE_SRC_RESOURCE(u, type)
#define SrcType uvec4
#else
#error "Not all source formats are accounted for"
#endif

#if SrcIs2D
#define SRC_RESOURCE_NAME texture2D
#elif SrcIs2DArray
#define SRC_RESOURCE_NAME texture2DArray
#elif SrcIs3D
#define SRC_RESOURCE_NAME texture3D
#elif SrcIsYUV
#define SRC_RESOURCE_NAME sampler2D
#elif SrcIs2DMS
#define SRC_RESOURCE_NAME texture2DMS
#else
#error "Not all source types are accounted for"
#endif

#if DstIsFloat
#define DstType vec4
#elif DstIsSint
#define DstType ivec4
#elif DstIsUint
#define DstType uvec4
#else
#error "Not all destination formats are accounted for"
#endif

layout(set = 0, binding = 0) uniform SRC_RESOURCE(SRC_RESOURCE_NAME) src;
layout(location = 0) out DstType dst;

#include "ImageCopy.inc"

void main()
{
    ivec2 srcSubImageCoords = transformImageCoords(ivec2(gl_FragCoord.xy));

#if SrcIs2D
    SrcType srcValue = texelFetch(src, params.srcOffset + srcSubImageCoords, params.srcMip);
#elif SrcIs2DArray || SrcIs3D
    SrcType srcValue = texelFetch(src, ivec3(params.srcOffset + srcSubImageCoords, params.srcLayer), params.srcMip);
#else
#error "Not all source types are accounted for"
#endif

    dst = transformSrcValue(srcValue);
}
